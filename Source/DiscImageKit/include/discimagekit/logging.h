#pragma once

#include <fmt/format.h>

#include "discimagekit/log.h"

namespace Common::Log
{

enum class LogType : int
{
  ACHIEVEMENTS,
  ACTIONREPLAY,
  AUDIO,
  AUDIO_INTERFACE,
  BOOT,
  COMMANDPROCESSOR,
  COMMON,
  CONSOLE,
  CONTROLLERINTERFACE,
  CORE,
  DISCIO,
  DSPHLE,
  DSPLLE,
  DSP_MAIL,
  DSPINTERFACE,
  DVDINTERFACE,
  DYNA_REC,
  EXPANSIONINTERFACE,
  FILEMON,
  FRAMEDUMP,
  GDB_STUB,
  GPFIFO,
  HOST_GPU,
  HSP,
  IOS,
  IOS_DI,
  IOS_ES,
  IOS_FS,
  IOS_NET,
  IOS_SD,
  IOS_SSL,
  IOS_STM,
  IOS_USB,
  IOS_WC24,
  IOS_WFS,
  IOS_WIIMOTE,
  MASTER_LOG,
  MEMMAP,
  MEMCARD_MANAGER,
  NETPLAY,
  OSHLE,
  OSREPORT,
  OSREPORT_HLE,
  PIXELENGINE,
  PROCESSORINTERFACE,
  POWERPC,
  SERIALINTERFACE,
  SP1,
  SYMBOLS,
  VIDEO,
  VIDEOINTERFACE,
  WII_IPC,
  WIIMOTE,

  NUMBER_OF_LOGS
};

enum class LogLevel : int
{
  LERROR = 2,
  LWARNING = 3,
  LINFO = 4,
  LDEBUG = 5,
  LNOTICE = 1,
};

inline constexpr auto MAX_LOGLEVEL = LogLevel::LINFO;

template <typename... Args>
void Log(LogLevel level, const char* format, Args&&... args)
{
  const std::string message =
      fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
  switch (level)
  {
  case LogLevel::LERROR:
    dik::log_err("%s", message.c_str());
    break;
  case LogLevel::LWARNING:
    dik::log_warn("%s", message.c_str());
    break;
  case LogLevel::LNOTICE:
  case LogLevel::LINFO:
  case LogLevel::LDEBUG:
  default:
    dik::log_info("%s", message.c_str());
    break;
  }
}
}  // namespace Common::Log

#define MASTER_LOG ::Common::Log::LogType::MASTER_LOG

#define ERROR_LOG_FMT(tag, format, ...)   ::Common::Log::Log(::Common::Log::LogLevel::LERROR, format __VA_OPT__(, ) __VA_ARGS__)
#define WARN_LOG_FMT(tag, format, ...)    ::Common::Log::Log(::Common::Log::LogLevel::LWARNING, format __VA_OPT__(, ) __VA_ARGS__)
#define NOTICE_LOG_FMT(tag, format, ...)  ::Common::Log::Log(::Common::Log::LogLevel::LNOTICE, format __VA_OPT__(, ) __VA_ARGS__)
#define INFO_LOG_FMT(tag, format, ...)    ::Common::Log::Log(::Common::Log::LogLevel::LINFO, format __VA_OPT__(, ) __VA_ARGS__)
#define DEBUG_LOG_FMT(tag, format, ...)   ::Common::Log::Log(::Common::Log::LogLevel::LDEBUG, format __VA_OPT__(, ) __VA_ARGS__)

