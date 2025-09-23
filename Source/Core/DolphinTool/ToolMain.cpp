// Copyright 2021 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#ifndef DIK_STANDALONE
#include "Common/StringUtil.h"
#include "Core/Core.h"
#endif
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include "DolphinTool/ConvertCommand.h"
#include "DolphinTool/ExtractCommand.h"
#include "DolphinTool/HeaderCommand.h"
#include "DolphinTool/VerifyCommand.h"

static void PrintUsage()
{
  fmt::print(std::cerr, "usage: dolphin-tool COMMAND -h\n"
                        "\n"
                        "commands supported: [convert, verify, header, extract]\n");
}

#ifdef _WIN32
#define main app_main
#endif

int main(int argc, char* argv[])
{
#ifndef DIK_STANDALONE
  Core::DeclareAsHostThread();
#endif

  if (argc < 2)
  {
    PrintUsage();
    return EXIT_FAILURE;
  }

  const std::string_view command_str = argv[1];
  // Take off the program name and command selector before passing arguments down
  const std::vector<std::string> args(argv + 2, argv + argc);

  if (command_str == "convert")
    return DolphinTool::ConvertCommand(args);
  else if (command_str == "verify")
    return DolphinTool::VerifyCommand(args);
  else if (command_str == "header")
    return DolphinTool::HeaderCommand(args);
  else if (command_str == "extract")
    return DolphinTool::Extract(args);
  PrintUsage();
  return EXIT_FAILURE;
}

#ifdef _WIN32
int wmain(int, wchar_t*[], wchar_t*[])
{
#ifdef DIK_STANDALONE
  int argc = 0;
  LPWSTR* argv_w = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (!argv_w)
    return EXIT_FAILURE;

  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 0; i < argc; ++i)
  {
    const int required_length = WideCharToMultiByte(CP_UTF8, 0, argv_w[i], -1, nullptr, 0, nullptr, nullptr);
    if (required_length <= 0)
    {
      LocalFree(argv_w);
      return EXIT_FAILURE;
    }

    std::string utf8(required_length - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, argv_w[i], -1, utf8.data(), required_length, nullptr, nullptr);
    args.emplace_back(std::move(utf8));
  }
  LocalFree(argv_w);

  std::vector<char*> argv;
  argv.reserve(args.size());
  for (std::string& arg : args)
    argv.push_back(arg.data());

  return main(static_cast<int>(argv.size()), argv.data());
#else
  std::vector<std::string> args = Common::CommandLineToUtf8Argv(GetCommandLineW());
  const int argc = static_cast<int>(args.size());
  std::vector<char*> argv(args.size());
  for (size_t i = 0; i < args.size(); ++i)
    argv[i] = args[i].data();

  return main(argc, argv.data());
#endif
}

#undef main
#endif
