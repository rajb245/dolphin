#include "discimagekit/fs_utils.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <ranges>
#include <utility>
#include <vector>

#include "discimagekit/log.h"

namespace
{
using PathArray =
    std::array<std::string, static_cast<std::size_t>(dik::fs::UserPath::Count)>;

PathArray g_paths;
std::once_flag g_path_once;

std::filesystem::path ToFsPath(std::string_view path)
{
  return std::filesystem::u8path(path);
}

std::string DefaultRoot()
{
  if (const char* home = std::getenv("HOME"))
    return (std::filesystem::path(home) / "dolphin-tool").string();
  return (std::filesystem::temp_directory_path() / "dolphin-tool").string();
}

void InitializePaths(const std::optional<std::string>& custom_root)
{
  const std::filesystem::path base =
      custom_root && !custom_root->empty() ? std::filesystem::path(*custom_root) :
                                             std::filesystem::path(DefaultRoot());

  g_paths.fill(base.string());
  g_paths[static_cast<std::size_t>(dik::fs::UserPath::User)] = base.string();
  g_paths[static_cast<std::size_t>(dik::fs::UserPath::WiiRoot)] =
      (base / dik::fs::wii_directory).string();
  g_paths[static_cast<std::size_t>(dik::fs::UserPath::WiiSessionRoot)] =
      (base / dik::fs::wii_session_directory).string();
  g_paths[static_cast<std::size_t>(dik::fs::UserPath::Cache)] =
      (base / dik::fs::cache_directory).string();
  g_paths[static_cast<std::size_t>(dik::fs::UserPath::RedumpCache)] =
      (base / dik::fs::cache_directory / dik::fs::redump_cache_directory).string();
  g_paths[static_cast<std::size_t>(dik::fs::UserPath::Logs)] =
      (base / dik::fs::logs_directory).string();
  g_paths[static_cast<std::size_t>(dik::fs::UserPath::WiiBanners)] =
      (base / dik::fs::wii_banners_directory).string();

  dik::log_info("DiscImageKit fs root: %s", base.generic_string().c_str());
}

void EnsurePathsInitialized(std::optional<std::string> custom_root)
{
  std::call_once(g_path_once, InitializePaths, custom_root);
}

std::filesystem::path ParentPath(std::string_view path)
{
  return ToFsPath(path).parent_path();
}

}  // namespace

namespace dik::fs
{

FileInfo::FileInfo(const std::string& path) : FileInfo(path.c_str()) {}

FileInfo::FileInfo(const char* path)
{
  const auto fs_path = ToFsPath(path);
  std::error_code ec;
  m_status = std::filesystem::status(fs_path, ec);
  m_size = std::filesystem::file_size(fs_path, ec);
  if (ec)
    m_size = 0;
  m_exists = std::filesystem::exists(m_status);
}

bool FileInfo::exists() const
{
  return m_exists;
}

bool FileInfo::is_directory() const
{
  return std::filesystem::is_directory(m_status);
}

bool FileInfo::is_file() const
{
  return exists() && !std::filesystem::is_directory(m_status);
}

u64 FileInfo::size() const
{
  if (!is_file())
    return 0;
  return static_cast<u64>(m_size);
}

void initialize(std::optional<std::string> custom_root)
{
  EnsurePathsInitialized(std::move(custom_root));
}

void set_path(UserPath key, std::string path)
{
  initialize();
  g_paths[static_cast<std::size_t>(key)] = std::move(path);
}

const std::string& get_path(UserPath key)
{
  initialize();
  return g_paths[static_cast<std::size_t>(key)];
}

bool exists(std::string_view path)
{
  return std::filesystem::exists(ToFsPath(path));
}

bool is_directory(std::string_view path)
{
  return std::filesystem::is_directory(ToFsPath(path));
}

bool is_file(std::string_view path)
{
  return std::filesystem::is_regular_file(ToFsPath(path));
}

u64 file_size(std::string_view path)
{
  std::error_code ec;
  const auto size = std::filesystem::file_size(ToFsPath(path), ec);
  return ec ? 0 : static_cast<u64>(size);
}

u64 file_size(std::FILE* file)
{
  if (!file)
    return 0;
  const auto current = std::ftell(file);
  if (current == -1)
    return 0;
  if (std::fseek(file, 0, SEEK_END) != 0)
    return 0;
  const auto end = std::ftell(file);
  std::fseek(file, current, SEEK_SET);
  return end < 0 ? 0 : static_cast<u64>(end);
}

bool create_dir(std::string_view path)
{
  std::error_code ec;
  std::filesystem::create_directory(ToFsPath(path), ec);
  return !ec || ec.value() == static_cast<int>(std::errc::file_exists);
}

bool create_dirs(std::string_view path)
{
  std::error_code ec;
  std::filesystem::create_directories(ToFsPath(path), ec);
  if (ec && ec.value() != static_cast<int>(std::errc::file_exists))
    return false;
  return true;
}

bool ensure_parent_dirs(std::string_view path)
{
  const auto parent = ParentPath(path);
  if (parent.empty())
    return true;
  return create_dirs(parent.generic_string());
}

bool remove_file(std::string_view path, IfAbsentBehavior)
{
  std::error_code ec;
  std::filesystem::remove(ToFsPath(path), ec);
  if (ec && ec.value() != static_cast<int>(std::errc::no_such_file_or_directory))
    return false;
  return true;
}

bool remove_dir(std::string_view path, IfAbsentBehavior)
{
  std::error_code ec;
  std::filesystem::remove(ToFsPath(path), ec);
  if (ec && ec.value() != static_cast<int>(std::errc::no_such_file_or_directory))
    return false;
  return true;
}

bool remove_dir_recursive(std::string_view path)
{
  std::error_code ec;
  std::filesystem::remove_all(ToFsPath(path), ec);
  return !ec;
}

bool rename_path(std::string_view from, std::string_view to)
{
  std::error_code ec;
  std::filesystem::rename(ToFsPath(from), ToFsPath(to), ec);
  return !ec;
}

bool rename_path_sync(std::string_view from, std::string_view to)
{
  return rename_path(from, to);
}

bool copy_file(std::string_view from, std::string_view to)
{
  std::error_code ec;
  std::filesystem::copy_file(ToFsPath(from), ToFsPath(to),
                             std::filesystem::copy_options::overwrite_existing, ec);
  return !ec;
}

bool create_empty_file(std::string_view path)
{
  std::ofstream file(ToFsPath(path));
  return file.good();
}

DirectoryEntry scan_directory_tree(std::string path, bool recursive)
{
  DirectoryEntry root;
  root.isDirectory = true;
  root.physicalName = path;
  root.virtualName = path;

  const auto base = ToFsPath(path);
  if (!std::filesystem::exists(base))
    return root;

  std::error_code ec;
  for (const auto& entry : std::filesystem::directory_iterator(base, ec))
  {
    DirectoryEntry child;
    child.physicalName = entry.path().generic_string();
    child.virtualName = entry.path().filename().generic_string();
    if (entry.is_directory(ec))
    {
      child.isDirectory = true;
      child.size = 0;
      if (recursive)
        child = scan_directory_tree(entry.path().generic_string(), true);
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

std::vector<std::string> scan_directory(
    const std::string& path, bool recursive,
    const std::function<bool(const std::filesystem::directory_entry&)>& filter)
{
  std::vector<std::string> results;
  std::error_code ec;
  if (recursive)
  {
    for (auto it = std::filesystem::recursive_directory_iterator(ToFsPath(path), ec);
         !ec && it != std::filesystem::recursive_directory_iterator(); ++it)
    {
      if (!filter || filter(*it))
        results.emplace_back(it->path().generic_string());
    }
  }
  else
  {
    for (auto it = std::filesystem::directory_iterator(ToFsPath(path), ec);
         !ec && it != std::filesystem::directory_iterator(); ++it)
    {
      if (!filter || filter(*it))
        results.emplace_back(it->path().generic_string());
    }
  }
  return results;
}

std::vector<std::string> glob_files(const std::vector<std::string>& roots,
                                    const std::vector<std::string>& extensions)
{
  std::vector<std::string> lowered_exts;
  lowered_exts.reserve(extensions.size());
  for (const auto& ext : extensions)
  {
    std::string lowered = ext;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
      return static_cast<char>(std::tolower(c));
    });
    lowered_exts.emplace_back(std::move(lowered));
  }

  auto matches_extension = [&](const std::filesystem::path& path) {
    if (lowered_exts.empty())
      return true;
    std::string ext = path.extension().generic_string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
      return static_cast<char>(std::tolower(c));
    });
    return std::ranges::find(lowered_exts, ext) != lowered_exts.end();
  };

  std::vector<std::string> results;
  for (const auto& root : roots)
  {
    auto matches = scan_directory(root, true, [&](const std::filesystem::directory_entry& entry) {
      return entry.is_regular_file() && matches_extension(entry.path());
    });
    results.insert(results.end(), matches.begin(), matches.end());
  }

  std::ranges::sort(results);
  const auto unique_end = std::ranges::unique(results).begin();
  results.erase(unique_end, results.end());
  return results;
}

}  // namespace dik::fs

namespace File
{

const std::string& GetUserPath(unsigned index)
{
  if (index >= NUM_PATH_INDICES)
    index = D_USER_IDX;
  return dik::fs::get_path(static_cast<dik::fs::UserPath>(index));
}

void SetUserPath(unsigned index, std::string path)
{
  if (index >= NUM_PATH_INDICES)
    index = D_USER_IDX;
  dik::fs::set_path(static_cast<dik::fs::UserPath>(index), std::move(path));
}

bool Exists(const std::string& path)
{
  return dik::fs::exists(path);
}

bool IsDirectory(const std::string& path)
{
  return dik::fs::is_directory(path);
}

bool IsFile(const std::string& path)
{
  return dik::fs::is_file(path);
}

u64 GetSize(const std::string& path)
{
  return dik::fs::file_size(path);
}

u64 GetSize(std::FILE* file)
{
  return dik::fs::file_size(file);
}

bool CreateDir(const std::string& path)
{
  return dik::fs::create_dir(path);
}

bool CreateDirs(std::string_view path)
{
  return dik::fs::create_dirs(path);
}

bool CreateFullPath(std::string_view full_path)
{
  return dik::fs::ensure_parent_dirs(full_path);
}

bool Delete(const std::string& path, IfAbsentBehavior behavior)
{
  return dik::fs::remove_file(path, behavior);
}

bool DeleteDir(const std::string& path, IfAbsentBehavior behavior)
{
  return dik::fs::remove_dir(path, behavior);
}

bool DeleteDirRecursively(const std::string& path)
{
  return dik::fs::remove_dir_recursive(path);
}

bool Rename(const std::string& from, const std::string& to)
{
  return dik::fs::rename_path(from, to);
}

bool RenameSync(const std::string& from, const std::string& to)
{
  return dik::fs::rename_path_sync(from, to);
}

bool CopyRegularFile(std::string_view from, std::string_view to)
{
  return dik::fs::copy_file(from, to);
}

bool CreateEmptyFile(const std::string& path)
{
  return dik::fs::create_empty_file(path);
}

FSTEntry ScanDirectoryTree(std::string path, bool recursive)
{
  return dik::fs::scan_directory_tree(std::move(path), recursive);
}

}  // namespace File
