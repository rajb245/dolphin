#include "Common/FileUtil.h"

#include <array>
#include <cstdlib>
#include <fstream>
#include <mutex>

#include "Common/CommonPaths.h"
#include "discimagekit/log.h"

namespace
{
using PathArray = std::array<std::string, NUM_PATH_INDICES>;
PathArray g_user_paths;
std::once_flag g_paths_once;

std::string DefaultUserRoot()
{
  namespace fs = std::filesystem;
  if (const char* env = std::getenv("HOME"))
    return (fs::path(env) / NORMAL_USER_DIR).string();
  return (fs::temp_directory_path() / NORMAL_USER_DIR).string();
}

void InitializeUserPaths()
{
  namespace fs = std::filesystem;
  const fs::path root = fs::path(DefaultUserRoot());
  g_user_paths.fill(root.string());

  auto set_path = [&](size_t idx, const fs::path& p) {
    if (idx < g_user_paths.size())
      g_user_paths[idx] = p.string();
  };

  set_path(D_USER_IDX, root);
  set_path(D_SESSION_WIIROOT_IDX, root / WII_USER_DIR);
  set_path(D_CACHE_IDX, root / CACHE_DIR);
  set_path(D_REDUMPCACHE_IDX, root / CACHE_DIR / REDUMPCACHE_DIR);
  set_path(D_LOGS_IDX, root / LOGS_DIR);
  set_path(F_MAINLOG_IDX, (root / LOGS_DIR / MAIN_LOG));
  set_path(D_BANNERS_WIIROOT_IDX, root / WIIBANNERS_DIR);

  dik::log_info("DiscImageKit user root: %s", g_user_paths[D_USER_IDX].c_str());
}

void EnsurePaths()
{
  std::call_once(g_paths_once, InitializeUserPaths);
}

std::filesystem::path ToFsPath(std::string_view path)
{
  return std::filesystem::u8path(path);
}

}  // namespace

namespace File
{

const std::string& GetUserPath(unsigned index)
{
  EnsurePaths();
  if (index >= g_user_paths.size())
    index = D_USER_IDX;
  return g_user_paths[index];
}

void SetUserPath(unsigned index, std::string path)
{
  EnsurePaths();
  if (index < g_user_paths.size())
    g_user_paths[index] = std::move(path);
}

bool Exists(const std::string& path)
{
  return std::filesystem::exists(ToFsPath(path));
}

bool IsDirectory(const std::string& path)
{
  return std::filesystem::is_directory(ToFsPath(path));
}

bool IsFile(const std::string& path)
{
  return std::filesystem::is_regular_file(ToFsPath(path));
}

u64 GetSize(const std::string& path)
{
  std::error_code ec;
  const auto size = std::filesystem::file_size(ToFsPath(path), ec);
  return ec ? 0 : static_cast<u64>(size);
}

u64 GetSize(FILE* f)
{
  if (!f)
    return 0;
  const auto current = std::ftell(f);
  if (current == -1)
    return 0;
  if (std::fseek(f, 0, SEEK_END) != 0)
    return 0;
  const auto end = std::ftell(f);
  std::fseek(f, current, SEEK_SET);
  return end < 0 ? 0 : static_cast<u64>(end);
}

bool CreateDir(const std::string& path)
{
  std::error_code ec;
  std::filesystem::create_directory(ToFsPath(path), ec);
  return !ec || ec.value() == static_cast<int>(std::errc::file_exists);
}

bool CreateDirs(std::string_view path)
{
  std::error_code ec;
  std::filesystem::create_directories(ToFsPath(path), ec);
  if (ec && ec.value() != static_cast<int>(std::errc::file_exists))
    return false;
  return true;
}

bool CreateFullPath(std::string_view full_path)
{
  auto dir = ToFsPath(full_path).parent_path();
  if (dir.empty())
    return true;
  const std::string dir_string = dir.generic_string();
  return CreateDirs(dir_string);
}

bool Delete(const std::string& filename, IfAbsentBehavior)
{
  std::error_code ec;
  std::filesystem::remove(ToFsPath(filename), ec);
  if (ec && ec.value() != static_cast<int>(std::errc::no_such_file_or_directory))
    return false;
  return true;
}

bool DeleteDir(const std::string& path, IfAbsentBehavior)
{
  std::error_code ec;
  std::filesystem::remove(ToFsPath(path), ec);
  if (ec && ec.value() != static_cast<int>(std::errc::no_such_file_or_directory))
    return false;
  return true;
}

bool DeleteDirRecursively(const std::string& path)
{
  std::error_code ec;
  std::filesystem::remove_all(ToFsPath(path), ec);
  return !ec;
}

bool Rename(const std::string& src, const std::string& dst)
{
  std::error_code ec;
  std::filesystem::rename(ToFsPath(src), ToFsPath(dst), ec);
  return !ec;
}

bool RenameSync(const std::string& src, const std::string& dst)
{
  return Rename(src, dst);
}

bool CopyRegularFile(std::string_view source, std::string_view destination)
{
  std::error_code ec;
  std::filesystem::copy_file(ToFsPath(source), ToFsPath(destination),
                             std::filesystem::copy_options::overwrite_existing, ec);
  return !ec;
}

bool CreateEmptyFile(const std::string& filename)
{
  std::ofstream file(ToFsPath(filename));
  return file.good();
}

FSTEntry ScanDirectoryTree(std::string directory, bool recursive)
{
  FSTEntry root;
  root.isDirectory = true;
  root.physicalName = directory;
  root.virtualName = directory;

  namespace fs = std::filesystem;
  const fs::path base = ToFsPath(directory);
  if (!fs::exists(base))
    return root;

  std::error_code ec;
  for (const auto& entry : fs::directory_iterator(base, ec))
  {
    FSTEntry child;
    child.physicalName = entry.path().string();
    child.virtualName = entry.path().filename().string();
    if (entry.is_directory(ec))
    {
      child.isDirectory = true;
      child.size = 0;
      if (recursive)
        child = ScanDirectoryTree(entry.path().string(), true);
      root.children.push_back(std::move(child));
    }
    else if (entry.is_regular_file(ec))
    {
      child.isDirectory = false;
      child.size = static_cast<u64>(entry.file_size(ec));
      root.children.push_back(std::move(child));
    }
  }
  return root;
}

}  // namespace File
