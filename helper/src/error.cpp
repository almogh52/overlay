#include <overlay/error.h>

namespace overlay {
namespace helper {

std::string GetErrorCodeDescription(ErrorCode code) {
  switch (code) {
    case ErrorCode::Success:
      return "Success";

    case ErrorCode::CoreDllNotFound:
      return "Unable to find the core DLL of the overlay";

    case ErrorCode::InvalidCoreDll:
      return "The overlay core DLL is invalid";

    case ErrorCode::ProcessNotFound:
      return "The requested process wasn't found";

    case ErrorCode::AlreadyConnected:
      return "The client is already connected to an overlay process";

    case ErrorCode::AuthFailed:
      return "Unable to authenticate to the overlay server";

    case ErrorCode::NotConnected:
      return "The client isn't connected to an overlay server";

    case ErrorCode::ClientObjectDeallocated:
      return "The client object has been deallocated";

    case ErrorCode::InvalidBitmapBufferSize:
      return "The bitmap buffer size is invalid (should be height * width * "
             "4(RGBA per pixel))";

    case ErrorCode::InvalidAttributes:
      return "One or more of the entered attributes contains invalid value";

    case ErrorCode::InjectorNotFound:
      return "The overlay injector executable wasn't found";

    case ErrorCode::InvalidEventType:
      return "The event type entered is invalid";

    case ErrorCode::InvalidCursor:
      return "The cursor type entered is invalid";

    default:
    case ErrorCode::UnknownError:
      return "Unknown Error";
  }
}

}  // namespace helper
}  // namespace overlay