#include <grpcpp/channel.h>
#include <grpcpp/grpcpp.h>
#include <overlay/client.h>
#include <overlay/error.h>
#include <windows.h>

#include <memory>
#include <string>

#include "auth.grpc.pb.h"
#include "authenticate_response.h"
#include "event_manager.h"
#include "utils/token.h"

namespace overlay {
namespace helper {

struct Client::Impl {
  Impl() : overlay_pid_(0), channel_(nullptr), event_manager_(nullptr) {}

  AuthenticateResponse GetAuthInfo() const;
  std::shared_ptr<grpc::Channel> ConnectToServerChannel() const;

  std::string FormatServerUrl(uint16_t port) const;

  std::shared_ptr<Event> GenerateEvent(EventReply &response) const;

  DWORD overlay_pid_;

  std::shared_ptr<grpc::Channel> channel_;

  std::unique_ptr<EventManager> event_manager_;
};

Client::Client() : pimpl_(new Impl) {}

Client::~Client() { delete pimpl_; }

void Client::ConnectToOverlay(DWORD pid) {
  // Check that the client isn't already connected to a client
  if (pimpl_->overlay_pid_ != 0) {
    throw Error(ErrorCode::AlreadyConnected);
  }

  pimpl_->overlay_pid_ = pid;

  // Connect to the server
  pimpl_->channel_ = pimpl_->ConnectToServerChannel();

  // Create event manager and start it
  pimpl_->event_manager_ = std::make_unique<EventManager>(pimpl_->channel_);
  pimpl_->event_manager_->StartHandlingAsyncRpcs();
}

void Client::SubscribeToEvent(
    EventType event_type,
    std::function<void(std::shared_ptr<Event>)> callback) {
  if (pimpl_->channel_ == nullptr) {
    throw Error(ErrorCode::NotConnected);
  }

  pimpl_->event_manager_->SubscribeToEvent(
      static_cast<EventReply::EventCase>(event_type),
      [this, callback](EventReply &response) {
        callback(pimpl_->GenerateEvent(response));
      });
}

void Client::UnsubscribeEvent(EventType event_type) {
  pimpl_->event_manager_->UnsubscribeEvent(
      static_cast<EventReply::EventCase>(event_type));
}

AuthenticateResponse Client::Impl::GetAuthInfo() const {
  AuthenticateResponse res;

  HANDLE pipe =
      CreateFileA(utils::token::GeneratePipeName(overlay_pid_).c_str(),
                  GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  DWORD read = 0;

  if (!pipe) {
    throw Error(ErrorCode::ProcessNotFound);
  }

  // Read the authenticate response from the server
  ReadFile(pipe, &res, sizeof(res), &read, NULL);
  CloseHandle(pipe);

  // Verify response size
  if (read != sizeof(res)) {
    throw Error(ErrorCode::UnknownError);
  }

  return res;
}

std::shared_ptr<grpc::Channel> Client::Impl::ConnectToServerChannel() const {
  std::shared_ptr<grpc::Channel> channel(nullptr);

  overlay::TokenAuthenticationRequest token_auth_req;
  overlay::TokenAuthenticationReply token_auth_res;
  std::unique_ptr<overlay::Authentication::Stub> auth_stub(nullptr);
  grpc::ClientContext context;

  AuthenticateResponse auth = GetAuthInfo();

  grpc::SslCredentialsOptions ssl_options;
  ssl_options.pem_root_certs = auth.server_certificate;

  // Create channel
  channel = grpc::CreateChannel(FormatServerUrl(auth.rpc_server_port),
                                grpc::SslCredentials(ssl_options));

  // Create authentication stub
  auth_stub = overlay::Authentication::NewStub(channel);

  // Authenticate with the server
  token_auth_req.set_token(
      std::string((const char *)&auth.token, sizeof(auth.token)));
  if (!auth_stub
           ->AuthenticateWithToken(&context, token_auth_req, &token_auth_res)
           .ok()) {
    throw Error(ErrorCode::AuthFailed);
  }

  return channel;
}

std::string Client::Impl::FormatServerUrl(uint16_t port) const {
  std::stringstream ss;

  ss << "localhost:" << port;

  return ss.str();
}

std::shared_ptr<Event> Client::Impl::GenerateEvent(EventReply &response) const {
  switch (response.event_case()) {
    case EventReply::EventCase::kFps:
      return std::shared_ptr<Event>(new FpsEvent(response.fps().fps()));

    default:
      return std::shared_ptr<Event>(nullptr);
  }
}

}  // namespace helper
}  // namespace overlay
