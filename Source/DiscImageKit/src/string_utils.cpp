#include "discimagekit/string_utils.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <codecvt>
#include <cstring>
#include <filesystem>
#include <locale>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "discimagekit/log.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <cerrno>
#include <iconv.h>
#endif

namespace dik::string_utils
{
namespace
{
#ifdef _WIN32
constexpr UINT SHIFT_JIS_CODEPAGE = 932;
constexpr UINT WINDOWS_1252_CODEPAGE = 1252;

std::wstring cp_to_utf16(UINT code_page, std::string_view input)
{
  if (input.empty())
    return {};

  const int required = MultiByteToWideChar(code_page, 0, input.data(), static_cast<int>(input.size()),
                                           nullptr, 0);
  if (required <= 0)
  {
    dik::log_warn("MultiByteToWideChar({}) failed with {}", code_page, GetLastError());
    return {};
  }

  std::wstring output(static_cast<size_t>(required), L'\0');
  const int converted = MultiByteToWideChar(code_page, 0, input.data(), static_cast<int>(input.size()),
                                            output.data(), required);
  if (converted != required)
  {
    dik::log_warn("MultiByteToWideChar({}) converted {} of {} characters", code_page, converted,
                  required);
    return {};
  }

  return output;
}

std::string utf16_to_cp(UINT code_page, std::wstring_view input)
{
  if (input.empty())
    return {};

  const int required =
      WideCharToMultiByte(code_page, 0, input.data(), static_cast<int>(input.size()), nullptr, 0,
                          nullptr, nullptr);
  if (required <= 0)
  {
    dik::log_warn("WideCharToMultiByte({}) failed with {}", code_page, GetLastError());
    return {};
  }

  std::string output(static_cast<size_t>(required), '\0');
  const int converted =
      WideCharToMultiByte(code_page, 0, input.data(), static_cast<int>(input.size()), output.data(),
                          required, nullptr, nullptr);
  if (converted != required)
  {
    dik::log_warn("WideCharToMultiByte({}) converted {} of {} characters", code_page, converted,
                  required);
    return {};
  }

  return output;
}

std::wstring utf8_to_wstring(std::string_view input)
{
  return cp_to_utf16(CP_UTF8, input);
}

std::string wstring_to_utf8(std::wstring_view input)
{
  return utf16_to_cp(CP_UTF8, input);
}

#else  // !_WIN32

std::string code_convert(const char* tocode, const char* fromcode, std::string_view input)
{
  if (input.empty())
    return {};

  const iconv_t descriptor = iconv_open(tocode, fromcode);
  if (descriptor == reinterpret_cast<iconv_t>(-1))
  {
    dik::log_warn("iconv_open {}<-{} failed: {}", tocode, fromcode, strerror(errno));
    return {};
  }

  const size_t in_bytes_total = input.size();
  const size_t out_buffer_size = std::max<size_t>(in_bytes_total * 4, 32);

  std::string output(out_buffer_size, '\0');
  auto* src_ptr = input.data();
  size_t src_bytes = in_bytes_total;
  auto* dst_ptr = output.data();
  size_t dst_bytes = output.size();

  while (src_bytes > 0)
  {
    const size_t result = iconv(descriptor, const_cast<char**>(&src_ptr), &src_bytes, &dst_ptr, &dst_bytes);
    if (result != static_cast<size_t>(-1))
      continue;

    if (errno == EILSEQ || errno == EINVAL)
    {
      // Skip invalid sequence.
      if (src_bytes > 0)
      {
        --src_bytes;
        ++src_ptr;
      }
    }
    else
    {
      dik::log_warn("iconv {}<-{} failed mid-conversion: {}", tocode, fromcode, strerror(errno));
      break;
    }
  }

  output.resize(out_buffer_size - dst_bytes);
  iconv_close(descriptor);
  return output;
}

std::string code_to_utf8(const char* fromcode, std::string_view input)
{
  return code_convert("UTF-8", fromcode, input);
}

#endif  // _WIN32

std::string utf16_to_utf8(std::u16string_view input)
{
  if (input.empty())
    return {};

  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
  return converter.to_bytes(input.data(), input.data() + input.size());
}

[[maybe_unused]] std::u16string utf8_to_utf16(std::string_view input)
{
  if (input.empty())
    return {};

  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
  return converter.from_bytes(input.data(), input.data() + input.size());
}

char16_t swap_bytes(char16_t value)
{
  return static_cast<char16_t>((value >> 8) | (value << 8));
}

std::mutex translator_mutex;
translator_t translator_callback = nullptr;

inline unsigned char to_unsigned(char c)
{
  return static_cast<unsigned char>(c);
}

}  // namespace

bool split_path(std::string_view full_path, std::string* path, std::string* filename,
                std::string* extension)
{
  if (full_path.empty())
    return false;

  size_t dir_end = full_path.find_last_of("/"
#ifdef _WIN32
                                         ":"
#endif
  );
  if (dir_end == std::string::npos)
  {
    dir_end = 0;
  }
  else
  {
    dir_end += 1;
  }

  size_t fname_end = full_path.rfind('.')
#ifdef _WIN32
                     ;
#else
                     ;
#endif
  if (fname_end == std::string::npos || fname_end < dir_end)
    fname_end = full_path.size();

  if (path)
    *path = std::string(full_path.substr(0, dir_end));
  if (filename)
    *filename = std::string(full_path.substr(dir_end, fname_end - dir_end));
  if (extension)
    *extension = std::string(full_path.substr(fname_end));

  return true;
}

std::string cp1252_to_utf8(std::string_view str)
{
#ifdef _WIN32
  return wstring_to_utf8(cp_to_utf16(WINDOWS_1252_CODEPAGE, str));
#else
  return code_to_utf8("CP1252", str);
#endif
}

std::string shift_jis_to_utf8(std::string_view str)
{
#ifdef _WIN32
  return wstring_to_utf8(cp_to_utf16(SHIFT_JIS_CODEPAGE, str));
#else
  return code_to_utf8("SJIS", str);
#endif
}

std::string utf8_to_shift_jis(std::string_view str)
{
#ifdef _WIN32
  return utf16_to_cp(SHIFT_JIS_CODEPAGE, utf8_to_wstring(str));
#else
  return code_convert("SJIS", "UTF-8", str);
#endif
}

std::string replace_all(std::string str, std::string_view src, std::string_view dest)
{
  if (src.empty())
    return str;

  size_t pos = 0;
  while ((pos = str.find(src, pos)) != std::string::npos)
  {
    str.replace(pos, src.size(), dest);
    pos += dest.size();
  }
  return str;
}

std::filesystem::path string_to_path(std::string_view path)
{
#ifdef _WIN32
  return std::filesystem::path(utf8_to_wstring(path));
#else
  return std::filesystem::path(path);
#endif
}

std::string path_to_string(const std::filesystem::path& path)
{
#ifdef _WIN32
  return wstring_to_utf8(path.native());
#else
  return path.native();
#endif
}

std::vector<std::string> split_string(std::string_view str, char delimiter)
{
  std::vector<std::string> result;
  std::string current;
  for (char ch : str)
  {
    if (ch == delimiter)
    {
      result.emplace_back(std::move(current));
      current.clear();
    }
    else
    {
      current.push_back(ch);
    }
  }
  result.emplace_back(std::move(current));

  if (!result.empty() && result.back().empty() && !str.empty() && str.back() == delimiter)
    result.pop_back();

  return result;
}

std::string_view strip_whitespace(std::string_view str)
{
  while (!str.empty() && std::isspace(to_unsigned(str.front()), std::locale::classic()))
    str.remove_prefix(1);
  while (!str.empty() && std::isspace(to_unsigned(str.back()), std::locale::classic()))
    str.remove_suffix(1);
  return str;
}

std::string utf16be_to_utf8(const char16_t* str, size_t max_size)
{
  if (str == nullptr || max_size == 0)
    return {};

  const char16_t* end = str;
  const char16_t* const limit = str + max_size;
  while (end < limit && *end != u'\0')
    ++end;

  std::u16string buffer;
  buffer.reserve(static_cast<size_t>(end - str));
  for (const char16_t* it = str; it != end; ++it)
  {
    char16_t value = *it;
#if defined(_WIN32) || (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) || \
    defined(__LITTLE_ENDIAN__)
    value = swap_bytes(value);
#endif
    buffer.push_back(value);
  }

  return utf16_to_utf8(buffer);
}

bool case_insensitive_equals(std::string_view a, std::string_view b)
{
  if (a.size() != b.size())
    return false;

  for (size_t i = 0; i < a.size(); ++i)
  {
    if (to_lower(a[i]) != to_lower(b[i]))
      return false;
  }
  return true;
}

char to_lower(char ch)
{
  return static_cast<char>(std::tolower(to_unsigned(ch), std::locale::classic()));
}

char to_upper(char ch)
{
  return static_cast<char>(std::toupper(to_unsigned(ch), std::locale::classic()));
}

void to_lower(std::string& str)
{
  std::transform(str.begin(), str.end(), str.begin(),
                 [](char ch) { return to_lower(ch); });
}

void to_upper(std::string& str)
{
  std::transform(str.begin(), str.end(), str.begin(),
                 [](char ch) { return to_upper(ch); });
}

bool is_printable(char c)
{
  return std::isprint(to_unsigned(c), std::locale::classic()) != 0;
}

bool is_alnum(char c)
{
  return std::isalnum(to_unsigned(c), std::locale::classic()) != 0;
}

void register_translator(translator_t translator)
{
  std::lock_guard<std::mutex> lock(translator_mutex);
  translator_callback = translator;
}

std::string translate(const char* string)
{
  if (!string)
    return {};

  std::lock_guard<std::mutex> lock(translator_mutex);
  if (translator_callback)
    return translator_callback(string);
  return std::string(string);
}

std::string bytes_to_hex(std::span<const u8> bytes)
{
  static constexpr std::array<char, 16> kHexDigits{
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  std::string result;
  result.reserve(bytes.size() * 2);
  for (const u8 value : bytes)
  {
    result.push_back(kHexDigits[(value >> 4) & 0xF]);
    result.push_back(kHexDigits[value & 0xF]);
  }
  return result;
}

}  // namespace dik::string_utils

std::filesystem::path StringToPath(std::string_view path)
{
  return dik::string_utils::string_to_path(path);
}

std::string PathToString(const std::filesystem::path& path)
{
  return dik::string_utils::path_to_string(path);
}

namespace Common
{
bool CaseInsensitiveEquals(std::string_view a, std::string_view b)
{
  return dik::string_utils::case_insensitive_equals(a, b);
}
}  // namespace Common
