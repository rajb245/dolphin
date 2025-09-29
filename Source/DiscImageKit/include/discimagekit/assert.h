#pragma once

#include <cstdlib>
#include <string_view>

#include <fmt/format.h>

#include "discimagekit/log.h"
#include "discimagekit/msg_handler.h"

namespace Common
{
[[noreturn]] inline void Crash()
{
  std::abort();
}

inline bool HandleAssertFailure(const char* expression, const char* file, int line,
                                const char* function, std::string_view message)
{
  const std::string full_message = fmt::format(
      "{}\n\n  Condition: {}\n  File: {}\n  Line: {}\n  Function: {}", message, expression,
      file, line, function);
  dik::log_err("Assertion failed: %s", full_message.c_str());
  return Common::MsgAlertFmt(true, Common::MsgType::Warning, "%s", full_message.c_str());
}

template <typename... Args>
inline void ReportAssertion(const char* expression, const char* file, int line,
                            const char* function, const char* format, Args&&... args)
{
  const std::string formatted =
      fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
  if (!HandleAssertFailure(expression, file, line, function, formatted))
    Crash();
}
}  // namespace Common

#define ASSERT_MSG(tag, condition, format, ...) \
  do \
  { \
    if (!(condition)) [[unlikely]] \
      Common::ReportAssertion(#condition, __FILE__, __LINE__, __func__, format __VA_OPT__(, ) __VA_ARGS__); \
  } while (0)

#define DEBUG_ASSERT_MSG(tag, condition, format, ...) ASSERT_MSG(tag, condition, format __VA_OPT__(, ) __VA_ARGS__)

#define ASSERT(condition) \
  do \
  { \
    if (!(condition)) [[unlikely]] \
      Common::ReportAssertion(#condition, __FILE__, __LINE__, __func__, "Assertion"); \
  } while (0)

#define DEBUG_ASSERT(condition) ASSERT(condition)
