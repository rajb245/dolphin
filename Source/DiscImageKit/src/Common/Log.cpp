#include "Common/Logging/Log.h"

#include <mutex>

#include <fmt/format.h>

#include "discimagekit/log.h"

namespace
{
std::mutex g_log_mutex;
}

namespace Common::Log
{
void GenericLogFmtImpl(LogLevel level, LogType type, const char* /*file*/, int /*line*/,
                       fmt::string_view format, const fmt::format_args& args)
{
  const std::string message = fmt::vformat(format, args);
  std::lock_guard<std::mutex> lock(g_log_mutex);
  switch (level)
  {
  case LogLevel::LERROR:
  case LogLevel::LWARNING:
    dik::log_warn("[%d] %s", static_cast<int>(type), message.c_str());
    break;
  case LogLevel::LNOTICE:
  case LogLevel::LINFO:
    dik::log_info("[%d] %s", static_cast<int>(type), message.c_str());
    break;
  case LogLevel::LDEBUG:
  default:
    dik::log_info("[DBG:%d] %s", static_cast<int>(type), message.c_str());
    break;
  }
}
}  // namespace Common::Log
