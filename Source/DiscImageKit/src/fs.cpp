#include "discimagekit/fs.h"

#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <string>

#include "discimagekit/fs_utils.h"
#include "discimagekit/log.h"

namespace dik
{
namespace
{
std::string g_user_directory;
std::once_flag g_user_directory_once;

std::string DetermineDefaultDirectory()
{
  namespace fs = std::filesystem;
  fs::path base;

  if (const char* env_home = std::getenv("HOME"))
  {
    base = fs::path(env_home);
  }
  else
  {
    base = fs::temp_directory_path();
  }

  base /= "dolphin-tool";
  return base.generic_string();
}

void InitializePaths(const std::string& base)
{
  File::SetUserPath(File::D_USER_IDX, base);
  File::CreateDirs(File::GetUserPath(File::D_USER_IDX));

  const std::string cache_root = base + std::string(1, dik::fs::dir_separator) +
                                 std::string(dik::fs::cache_directory);
  File::SetUserPath(File::D_CACHE_IDX, cache_root);
  File::CreateDirs(File::GetUserPath(File::D_CACHE_IDX));

  const std::string redump_cache = cache_root + std::string(1, dik::fs::dir_separator) +
                                   std::string(dik::fs::redump_cache_directory);
  File::SetUserPath(File::D_REDUMPCACHE_IDX, redump_cache);
  File::CreateDirs(File::GetUserPath(File::D_REDUMPCACHE_IDX));
}

void InitializeUserDirectoryOnce(const std::string& custom_path)
{
  std::string base = custom_path.empty() ? DetermineDefaultDirectory() : custom_path;
  InitializePaths(base);
  g_user_directory = File::GetUserPath(File::D_USER_IDX);
  log_info("DiscImageKit user directory: %s", g_user_directory.c_str());
}
}  // namespace

const std::string& user_directory()
{
  return g_user_directory;
}

void initialize_user_directory(const std::string& custom_path)
{
  std::call_once(g_user_directory_once, InitializeUserDirectoryOnce, custom_path);
}

}  // namespace dik
