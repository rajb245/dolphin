#include "Common/MsgHandler.h"

#include <atomic>
#include <cstdlib>
#include <mutex>

#include <fmt/format.h>

#include "Common/Logging/Log.h"
#include "discimagekit/log.h"

namespace
{
std::mutex g_alert_mutex;
Common::MsgAlertHandler g_alert_handler = nullptr;
Common::StringTranslator g_string_translator = nullptr;
std::atomic<bool> g_alerts_enabled{true};
std::atomic<bool> g_abort_on_panic{false};
}

namespace Common
{
void RegisterMsgAlertHandler(MsgAlertHandler handler)
{
  std::lock_guard<std::mutex> lock(g_alert_mutex);
  g_alert_handler = handler;
}

void RegisterStringTranslator(StringTranslator translator)
{
  std::lock_guard<std::mutex> lock(g_alert_mutex);
  g_string_translator = translator;
}

std::string GetStringT(const char* string)
{
  std::lock_guard<std::mutex> lock(g_alert_mutex);
  if (g_string_translator)
    return g_string_translator(string);
  return string ? std::string(string) : std::string();
}

bool MsgAlertFmtImpl(bool yes_no, MsgType style, Common::Log::LogType /*log_type*/, const char* /*file*/,
                     int /*line*/, fmt::string_view format, const fmt::format_args& args)
{
  const std::string formatted = fmt::vformat(format, args);

  if (!g_alerts_enabled.load(std::memory_order_relaxed))
    return false;

  std::lock_guard<std::mutex> lock(g_alert_mutex);
  if (g_alert_handler)
  {
    return g_alert_handler("DiscImageKit", formatted.c_str(), yes_no, style);
  }

  // Fall back to stderr/stdout logging.
  if (yes_no)
  {
    dik::log_warn("[Alert:%d] %s", static_cast<int>(style), formatted.c_str());
    return false;
  }

  if (style == MsgType::Critical)
    dik::log_err("[Alert:%d] %s", static_cast<int>(style), formatted.c_str());
  else
    dik::log_warn("[Alert:%d] %s", static_cast<int>(style), formatted.c_str());

  if (style == MsgType::Critical && g_abort_on_panic.load(std::memory_order_relaxed))
    std::abort();

  return false;
}

void SetEnableAlert(bool enable)
{
  g_alerts_enabled.store(enable, std::memory_order_relaxed);
}

void SetAbortOnPanicAlert(bool should_abort)
{
  g_abort_on_panic.store(should_abort, std::memory_order_relaxed);
}

}  // namespace Common
