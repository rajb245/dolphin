#pragma once

#include <fmt/format.h>
#include <string>

#include "discimagekit/logging.h"

namespace Common
{

enum class MsgType
{
  Information,
  Question,
  Warning,
  Critical
};

using MsgAlertHandler = bool (*)(const char* caption, const char* text, bool yes_no, MsgType style);
using StringTranslator = std::string (*)(const char* text);

void RegisterMsgAlertHandler(MsgAlertHandler handler);
void RegisterStringTranslator(StringTranslator translator);

[[nodiscard]] std::string GetStringT(const char* string);

bool MsgAlertFmtImpl(bool yes_no, MsgType style, fmt::string_view format,
                     const fmt::format_args& args);
bool MsgAlertFmtImpl(bool yes_no, MsgType style, Common::Log::LogType log_type, const char* file,
                     int line, fmt::string_view format, const fmt::format_args& args);

template <typename... Args>
inline bool MsgAlertFmt(bool yes_no, MsgType style, const char* format, Args&&... args)
{
  return MsgAlertFmtImpl(yes_no, style, format, fmt::make_format_args(args...));
}

template <typename... Args>
inline bool MsgAlertFmtT(bool yes_no, MsgType style, const char* format, Args&&... args)
{
  const std::string translated = GetStringT(format);
  return MsgAlertFmtImpl(yes_no, style, translated, fmt::make_format_args(args...));
}

void SetEnableAlert(bool enable);
void SetAbortOnPanicAlert(bool should_abort);

}  // namespace Common

#define GenericAlertFmt(yes_no, style, log_type, format, ...)                                        Common::MsgAlertFmt(yes_no, style, format __VA_OPT__(, ) __VA_ARGS__)
#define GenericAlertFmtT(yes_no, style, log_type, format, ...)                                       Common::MsgAlertFmtT(yes_no, style, format __VA_OPT__(, ) __VA_ARGS__)
#define SuccessAlertFmt(format, ...)                                                                 GenericAlertFmt(false, Common::MsgType::Information, MASTER_LOG,                                                   format __VA_OPT__(, ) __VA_ARGS__)
#define PanicAlertFmt(format, ...)                                                                   GenericAlertFmt(false, Common::MsgType::Warning, MASTER_LOG, format __VA_OPT__(, ) __VA_ARGS__)
#define PanicYesNoFmt(format, ...)                                                                   GenericAlertFmt(true, Common::MsgType::Warning, MASTER_LOG, format __VA_OPT__(, ) __VA_ARGS__)
#define AskYesNoFmt(format, ...)                                                                     GenericAlertFmt(true, Common::MsgType::Question, MASTER_LOG, format __VA_OPT__(, ) __VA_ARGS__)
#define CriticalAlertFmt(format, ...)                                                                GenericAlertFmt(false, Common::MsgType::Critical, MASTER_LOG, format __VA_OPT__(, ) __VA_ARGS__)
#define SuccessAlertFmtT(format, ...)                                                                GenericAlertFmtT(false, Common::MsgType::Information, MASTER_LOG,                                                   format __VA_OPT__(, ) __VA_ARGS__)
#define PanicAlertFmtT(format, ...)                                                                  GenericAlertFmtT(false, Common::MsgType::Warning, MASTER_LOG, format __VA_OPT__(, ) __VA_ARGS__)
#define PanicYesNoFmtT(format, ...)                                                                  GenericAlertFmtT(true, Common::MsgType::Warning, MASTER_LOG, format __VA_OPT__(, ) __VA_ARGS__)
#define AskYesNoFmtT(format, ...)                                                                    GenericAlertFmtT(true, Common::MsgType::Question, MASTER_LOG, format __VA_OPT__(, ) __VA_ARGS__)
#define CriticalAlertFmtT(format, ...)                                                               GenericAlertFmtT(false, Common::MsgType::Critical, MASTER_LOG, format __VA_OPT__(, ) __VA_ARGS__)
#define PanicYesNoFmtAssert(log_type, format, ...)                                                   GenericAlertFmt(true, Common::MsgType::Warning, log_type, format __VA_OPT__(, ) __VA_ARGS__)

