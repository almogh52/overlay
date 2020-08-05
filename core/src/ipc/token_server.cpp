#include "token_server.h"

#include <algorithm>
#include <loguru.hpp>
#include <sstream>

#include "authenticate_response.h"

namespace overlay {
namespace core {
namespace ipc {

TokenServer::TokenServer() : pipe_(INVALID_HANDLE_VALUE) {}

void TokenServer::StartTokenGeneratorServer(uint16_t rpc_server_port) {
  std::string pipe_name = GeneratePipeName();

  // Create named pipe for outbound communication
  pipe_ = CreateNamedPipeA(pipe_name.c_str(), PIPE_ACCESS_OUTBOUND,
                           PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE |
                               PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                           1, sizeof(AuthenticateResponse), 0, 0, NULL);
  CHECK_F(pipe_ != INVALID_HANDLE_VALUE,
          "Unable to create pipe for token generator server! (Error: 0x%x)",
          GetLastError());

  LOG_F(INFO,
        "Started token generator server with pipe name '%s'. (HANDLE: 0x%x)",
        pipe_name.c_str(), pipe_);

  // Start pipe main thread
  main_thread_ =
      std::thread(&TokenServer::PipeMainThread, this, rpc_server_port);
}

DWORD TokenServer::GetTokenProcessId(GUID token) {
  std::lock_guard tokens_lk(tokens_mutex_);

  if (!tokens_.count(token)) {
    return 0;
  }

  return tokens_[token];
}

void TokenServer::InvalidateProcessToken(GUID token) {
  std::lock_guard tokens_lk(tokens_mutex_);

  tokens_.erase(token);
}

void TokenServer::PipeMainThread(uint16_t rpc_server_port) {
  AuthenticateResponse response = {0};
  DWORD written_bytes = 0;

  DWORD process_id;

  std::unique_lock tokens_lk(tokens_mutex_, std::defer_lock);

  response.rpc_server_port = rpc_server_port;

  // TODO: Handle stopping threads
  while (true) {
    // If a client is connected
    if (ConnectNamedPipe(pipe_, NULL) ||
        GetLastError() == ERROR_PIPE_CONNECTED) {
      // Get client's process id
      if (GetNamedPipeClientProcessId(pipe_, &process_id)) {
        // TODO: Add verification for processes

        // Generate token for the process
        response.token = GenerateTokenForProcess(process_id);

        // Send the response to the client
        WriteFile(pipe_, &response, sizeof(response), &written_bytes, NULL);
      }
    }

    DisconnectNamedPipe(pipe_);
  }
}

std::string TokenServer::GeneratePipeName() const {
  std::stringstream ss;

  ss << "\\\\.\\pipe\\" << PIPE_IDENTIFIER << "-" << GetCurrentProcessId();

  return ss.str();
}

GUID TokenServer::GenerateTokenForProcess(DWORD pid) {
  GUID token;

  std::lock_guard tokens_lk(tokens_mutex_);

  // Check if a token already exists for the process
  for (auto it = tokens_.begin(); it != tokens_.end(); it++) {
    if (it->second == pid) {
      return it->first;
    }
  }

  // Generate random guid
  CoCreateGuid(&token);

  // Save token
  tokens_[token] = pid;

  LOG_F(INFO, "Generated token '%s' for process %d.",
        TokenToString(&token).c_str(), pid);

  return token;
}

std::string TokenServer::TokenToString(GUID *token) {
  char guid_string[37];  // 32 hex chars + 4 hyphens + null terminator

  snprintf(guid_string, sizeof(guid_string),
           "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", token->Data1,
           token->Data2, token->Data3, token->Data4[0], token->Data4[1],
           token->Data4[2], token->Data4[3], token->Data4[4], token->Data4[5],
           token->Data4[6], token->Data4[7]);

  return guid_string;
}

}  // namespace ipc
}  // namespace core
}  // namespace overlay