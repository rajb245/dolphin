#include "discimagekit/fs.h"

#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <string>

#include "Common/CommonPaths.h"
#include "Common/FileUtil.h"
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

void EnsureDirectory(const std::string& path)
{
  File::CreateFullPath(path + DIR_SEP);
}

void InitializePaths(const std::string& base)
{
  File::SetUserPath(D_USER_IDX, base);
  EnsureDirectory(File::GetUserPath(D_USER_IDX));

  const std::string cache_root = base + DIR_SEP + CACHE_DIR;
  File::SetUserPath(D_CACHE_IDX, cache_root);
  EnsureDirectory(File::GetUserPath(D_CACHE_IDX));

  const std::string redump_cache = cache_root + DIR_SEP + REDUMPCACHE_DIR;
  File::SetUserPath(D_REDUMPCACHE_IDX, redump_cache);
  EnsureDirectory(File::GetUserPath(D_REDUMPCACHE_IDX));
}

void InitializeUserDirectoryOnce(const std::string& custom_path)
{
  std::string base = custom_path.empty() ? DetermineDefaultDirectory() : custom_path;
  InitializePaths(base);
  g_user_directory = File::GetUserPath(D_USER_IDX);
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
