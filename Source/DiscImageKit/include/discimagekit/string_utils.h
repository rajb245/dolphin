#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "discimagekit/types.h"

namespace dik::string_utils
{
bool split_path(std::string_view full_path, std::string* path, std::string* filename,
               std::string* extension);

std::string cp1252_to_utf8(std::string_view str);
std::string shift_jis_to_utf8(std::string_view str);
std::string utf16be_to_utf8(const char16_t* str, size_t max_size);
std::string utf8_to_shift_jis(std::string_view str);

std::string replace_all(std::string str, std::string_view src, std::string_view dest);
std::filesystem::path string_to_path(std::string_view path);
std::string path_to_string(const std::filesystem::path& path);
std::vector<std::string> split_string(std::string_view str, char delimiter);
std::string_view strip_whitespace(std::string_view str);

bool case_insensitive_equals(std::string_view a, std::string_view b);

char to_lower(char ch);
char to_upper(char ch);
void to_lower(std::string& str);
void to_upper(std::string& str);

bool is_printable(char c);
bool is_alnum(char c);

std::string translate(const char* string);
using translator_t = std::string (*)(const char*);
void register_translator(translator_t translator);

template <typename... Args>
std::string format_localized(const char* format, Args&&... args)
{
  return fmt::format(fmt::runtime(translate(format)), std::forward<Args>(args)...);
}

std::string bytes_to_hex(std::span<const u8> bytes);
}

std::filesystem::path StringToPath(std::string_view path);
std::string PathToString(const std::filesystem::path& path);

namespace Common
{
bool CaseInsensitiveEquals(std::string_view a, std::string_view b);
}  // namespace Common
