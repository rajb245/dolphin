#include "Common/StringUtil.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <codecvt>
#include <cstring>
#include <filesystem>
#include <locale>
#include <type_traits>

#include "discimagekit/log.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <iconv.h>
#endif

namespace
{
constexpr u32 CODEPAGE_SHIFT_JIS = 932;
constexpr u32 CODEPAGE_WINDOWS_1252 = 1252;

std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>& Utf16Converter()
{
  static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  return conv;
}

const std::array<int, 128> kCp1252Table = {
    0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160,
    0x2039, 0x0152, 0x0000, 0x017D, 0x0000, 0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022,
    0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x017E, 0x0178, 0x00A0,
    0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB,
    0x00AC, 0x00AD, 0x00AE, 0x00AF, 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6,
    0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF, 0x00C0, 0x00C1,
    0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC,
    0x00CD, 0x00CE, 0x00CF, 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 0x00E0, 0x00E1, 0x00E2,
    0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED,
    0x00EE, 0x00EF, 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8,
    0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF};

std::string ManualCP1252ToUTF8(std::string_view str)
{
  std::string result;
  result.reserve(str.size());
  for (unsigned char c : str)
  {
    if (c < 0x80)
    {
      result.push_back(static_cast<char>(c));
    }
    else
    {
      const int code_point = kCp1252Table[c - 0x80];
      if (code_point == 0)
        continue;
      if (code_point < 0x800)
      {
        result.push_back(static_cast<char>(0xC0 | (code_point >> 6)));
        result.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
      }
      else
      {
        result.push_back(static_cast<char>(0xE0 | (code_point >> 12)));
        result.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
      }
    }
  }
  return result;
}

#ifdef _WIN32
std::wstring CPToUTF16(u32 code_page, std::string_view input)
{
  if (input.empty())
    return {};

  const int required = MultiByteToWideChar(code_page, 0, input.data(), static_cast<int>(input.size()),
                                           nullptr, 0);
  if (required <= 0)
    return {};

  std::wstring output(static_cast<size_t>(required), L'\0');
  const int converted = MultiByteToWideChar(code_page, 0, input.data(), static_cast<int>(input.size()),
                                            output.data(), required);
  if (converted != required)
    return {};

  return output;
}

std::string UTF16ToCP(u32 code_page, std::wstring_view input)
{
  if (input.empty())
    return {};

  const int required =
      WideCharToMultiByte(code_page, 0, input.data(), static_cast<int>(input.size()), nullptr, 0,
                          nullptr, nullptr);
  if (required <= 0)
    return {};

  std::string output(static_cast<size_t>(required), '\0');
  const int converted = WideCharToMultiByte(code_page, 0, input.data(), static_cast<int>(input.size()),
                                            output.data(), required, nullptr, nullptr);
  if (converted != required)
    return {};

  return output;
}
#else
std::string ConvertEncoding(const char* tocode, const char* fromcode, std::string_view input)
{
  if (input.empty())
    return {};

  iconv_t cd = iconv_open(tocode, fromcode);
  if (cd == (iconv_t)-1)
  {
    dik::log_warn("iconv_open(%s->%s) failed: %s", fromcode, tocode, strerror(errno));
    return {};
  }

  size_t in_remaining = input.size();
  const char* in_ptr = input.data();
  size_t out_capacity = (in_remaining ? in_remaining : 1) * 4 + 4;
  std::string output(out_capacity, '\0');
  char* out_ptr = output.data();
  size_t out_remaining = output.size();

  while (in_remaining > 0)
  {
    const size_t result = iconv(cd, const_cast<char**>(&in_ptr), &in_remaining, &out_ptr, &out_remaining);
    if (result == static_cast<size_t>(-1))
    {
      if (errno == EILSEQ || errno == EINVAL)
      {
        ++in_ptr;
        --in_remaining;
        continue;
      }

      if (errno == E2BIG)
      {
        const size_t produced = output.size() - out_remaining;
        output.resize(output.size() * 2);
        out_ptr = output.data() + produced;
        out_remaining = output.size() - produced;
        continue;
      }

      dik::log_warn("iconv(%s->%s) failed: %s", fromcode, tocode, strerror(errno));
      break;
    }
  }

  iconv_close(cd);
  const size_t produced = output.size() - out_remaining;
  output.resize(produced);
  return output;
}
#endif
}  // namespace

std::string_view StripWhitespace(std::string_view s)
{
  size_t start = 0;
  while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
    ++start;
  size_t end = s.size();
  while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
    --end;
  return s.substr(start, end - start);
}

std::string_view StripSpaces(std::string_view s)
{
  size_t start = 0;
  while (start < s.size() && s[start] == ' ')
    ++start;
  size_t end = s.size();
  while (end > start && s[end - 1] == ' ')
    --end;
  return s.substr(start, end - start);
}

std::string_view StripQuotes(std::string_view s)
{
  if (!s.empty() && s.front() == '"')
    s.remove_prefix(1);
  if (!s.empty() && s.back() == '"')
    s.remove_suffix(1);
  return s;
}

std::string ReplaceAll(std::string result, std::string_view src, std::string_view dest)
{
  size_t pos = 0;
  while ((pos = result.find(src, pos)) != std::string::npos)
  {
    result.replace(pos, src.size(), dest);
    pos += dest.size();
  }
  return result;
}

void ReplaceBreaksWithSpaces(std::string& str)
{
  std::replace(str.begin(), str.end(), '\n', ' ');
  std::replace(str.begin(), str.end(), '\r', ' ');
}

std::vector<std::string> SplitString(const std::string& str, char delim)
{
  std::vector<std::string> result;
  size_t start = 0;
  while (start <= str.size())
  {
    const size_t pos = str.find(delim, start);
    const size_t len = (pos == std::string::npos) ? std::string::npos : pos - start;
    result.emplace_back(str.substr(start, len));
    if (pos == std::string::npos)
      break;
    start = pos + 1;
  }
  return result;
}

#ifdef _WIN32
std::wstring UTF8ToWString(std::string_view input)
{
  return CPToUTF16(CP_UTF8, input);
}

std::string WStringToUTF8(std::wstring_view input)
{
  return UTF16ToCP(CP_UTF8, input);
}
#else
std::string WStringToUTF8(std::wstring_view input)
{
  using converter = std::conditional_t<sizeof(wchar_t) == 2, std::codecvt_utf8_utf16<wchar_t>,
                                       std::codecvt_utf8<wchar_t>>;
  std::wstring_convert<converter, wchar_t> conv;
  return conv.to_bytes(input.data(), input.data() + input.size());
}
#endif

std::string UTF16BEToUTF8(const char16_t* str, size_t max_size)
{
  if (!str)
    return {};
  std::u16string buffer;
  buffer.reserve(max_size);
  for (size_t i = 0; i < max_size && str[i] != 0; ++i)
  {
    const char16_t c = static_cast<char16_t>(((str[i] & 0xFF) << 8) | ((str[i] >> 8) & 0xFF));
    buffer.push_back(c);
  }
  return Utf16Converter().to_bytes(buffer);
}

std::string UTF8ToSHIFTJIS(std::string_view str)
{
#ifdef _WIN32
  return UTF16ToCP(CODEPAGE_SHIFT_JIS, UTF8ToWString(str));
#else
  const std::string converted = ConvertEncoding("SJIS", "UTF-8", str);
  return (!converted.empty() || str.empty()) ? converted : std::string(str);
#endif
}

std::string SHIFTJISToUTF8(std::string_view str)
{
#ifdef _WIN32
  return WStringToUTF8(CPToUTF16(CODEPAGE_SHIFT_JIS, str));
#else
  const std::string converted = ConvertEncoding("UTF-8", "SJIS", str);
  return (!converted.empty() || str.empty()) ? converted : std::string(str);
#endif
}

std::string CP1252ToUTF8(std::string_view str)
{
#ifdef _WIN32
  return WStringToUTF8(CPToUTF16(CODEPAGE_WINDOWS_1252, str));
#else
  const std::string converted = ConvertEncoding("UTF-8", "CP1252", str);
  if (!converted.empty() || str.empty())
    return converted;
  return ManualCP1252ToUTF8(str);
#endif
}

void UnifyPathSeparators(std::string& path)
{
  std::replace(path.begin(), path.end(), '\\', '/');
}

std::string WithUnifiedPathSeparators(std::string path)
{
  UnifyPathSeparators(path);
  return path;
}

std::string PathToFileName(std::string_view path)
{
  return std::filesystem::path(path).filename().string();
}

std::filesystem::path StringToPath(std::string_view path)
{
#ifdef _WIN32
  return std::filesystem::path(UTF8ToWString(path));
#else
  return std::filesystem::u8path(path);
#endif
}

std::string PathToString(const std::filesystem::path& path)
{
#ifdef _WIN32
  return WStringToUTF8(path.native());
#else
  return path.generic_string();
#endif
}

bool SplitPath(std::string_view full_path, std::string* dir, std::string* file,
               std::string* extension)
{
  std::filesystem::path p = std::filesystem::u8path(full_path);
  if (dir)
    *dir = p.parent_path().generic_string();
  if (file)
    *file = p.stem().generic_string();
  if (extension)
    *extension = p.extension().generic_string();
  return true;
}

namespace Common
{

bool CaseInsensitiveEquals(std::string_view lhs, std::string_view rhs)
{
  if (lhs.size() != rhs.size())
    return false;
  for (size_t i = 0; i < lhs.size(); ++i)
  {
    if (std::tolower(static_cast<unsigned char>(lhs[i])) !=
        std::tolower(static_cast<unsigned char>(rhs[i])))
    {
      return false;
    }
  }
  return true;
}

void ToUpper(std::string* value)
{
  if (!value)
    return;
  std::transform(value->begin(), value->end(), value->begin(), [](unsigned char c) {
    return static_cast<char>(std::toupper(c));
  });
}

void ToLower(std::string* value)
{
  if (!value)
    return;
  std::transform(value->begin(), value->end(), value->begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
}

}  // namespace Common
