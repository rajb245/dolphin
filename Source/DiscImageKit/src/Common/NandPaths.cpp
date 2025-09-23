#include "Common/NandPaths.h"

#include <fmt/format.h>

#include "Common/CommonPaths.h"
#include "Common/FileUtil.h"

namespace
{
std::string TitleBase(u64 title_id)
{
  return fmt::format("title/{:08x}/{:08x}", static_cast<unsigned>(title_id >> 32),
                     static_cast<unsigned>(title_id & 0xFFFFFFFFu));
}

std::string ResolveRoot(std::optional<Common::FromWhichRoot> from)
{
  using Common::FromWhichRoot;
  const auto index = [&]() {
    if (!from)
      return D_SESSION_WIIROOT_IDX;
    switch (*from)
    {
    case FromWhichRoot::Configured:
      return D_WIIROOT_IDX;
    case FromWhichRoot::Session:
      return D_SESSION_WIIROOT_IDX;
    case FromWhichRoot::Banners:
      return D_BANNERS_WIIROOT_IDX;
    }
    return D_SESSION_WIIROOT_IDX;
  }();
  return File::GetUserPath(index);
}
}

namespace Common
{
std::string RootUserPath(FromWhichRoot from)
{
  switch (from)
  {
  case FromWhichRoot::Configured:
    return File::GetUserPath(D_WIIROOT_IDX);
  case FromWhichRoot::Session:
    return File::GetUserPath(D_SESSION_WIIROOT_IDX);
  case FromWhichRoot::Banners:
    return File::GetUserPath(D_BANNERS_WIIROOT_IDX);
  }
  return File::GetUserPath(D_SESSION_WIIROOT_IDX);
}

std::string GetTitleDataPath(u64 title_id, std::optional<FromWhichRoot> from)
{
  return fmt::format("{}/{}{}", ResolveRoot(from), TitleBase(title_id), "/data");
}

std::string GetTitlePath(u64 title_id, std::optional<FromWhichRoot> from)
{
  return fmt::format("{}/{}", ResolveRoot(from), TitleBase(title_id));
}

std::string GetTitleContentPath(u64 title_id, std::optional<FromWhichRoot> from)
{
  return fmt::format("{}/{}{}", ResolveRoot(from), TitleBase(title_id), "/content");
}

std::string GetImportTitlePath(u64 title_id, std::optional<FromWhichRoot> from)
{
  return fmt::format("{}/import/{:08x}/{:08x}", ResolveRoot(from),
                     static_cast<unsigned>(title_id >> 32),
                     static_cast<unsigned>(title_id & 0xFFFFFFFFu));
}

std::string GetTicketFileName(u64 title_id, std::optional<FromWhichRoot> from)
{
  return fmt::format("{}/ticket/{:08x}/{:08x}/title.tik", ResolveRoot(from),
                     static_cast<unsigned>(title_id >> 32),
                     static_cast<unsigned>(title_id & 0xFFFFFFFFu));
}

std::string GetV1TicketFileName(u64 title_id, std::optional<FromWhichRoot> from)
{
  return GetTicketFileName(title_id, from);
}

std::string GetTMDFileName(u64 title_id, std::optional<FromWhichRoot> from)
{
  return GetTitleContentPath(title_id, from) + "/title.tmd";
}

std::string GetMiiDatabasePath(std::optional<FromWhichRoot> from)
{
  return fmt::format("{}/shared2/menu/FaceLib/RFL_DB.dat", ResolveRoot(from));
}

bool IsTitlePath(const std::string& path, std::optional<FromWhichRoot> from, u64* title_id)
{
  const std::string root = ResolveRoot(from);
  if (path.rfind(root, 0) != 0)
    return false;
  if (title_id)
    *title_id = 0;
  return true;
}

std::string EscapeFileName(const std::string& filename)
{
  return filename;
}

std::string EscapePath(const std::string& path)
{
  return path;
}

std::string UnescapeFileName(const std::string& filename)
{
  return filename;
}

bool IsFileNameSafe(const std::string_view filename)
{
  return filename.find_first_of("\\/") == std::string_view::npos;
}

}  // namespace Common
