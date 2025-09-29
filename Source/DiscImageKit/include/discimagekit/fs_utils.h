#pragma once

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "discimagekit/types.h"

namespace dik::fs
{

inline constexpr char dir_separator = '/';
inline constexpr std::string_view cache_directory = "Cache";
inline constexpr std::string_view redump_cache_directory = "Redump";
inline constexpr std::string_view logs_directory = "Logs";
inline constexpr std::string_view wii_directory = "Wii";
inline constexpr std::string_view wii_session_directory = "WiiSession";
inline constexpr std::string_view wii_banners_directory = "WiiBanners";

enum class UserPath : std::size_t
{
  User = 0,
  WiiRoot,
  WiiSessionRoot,
  Cache,
  RedumpCache,
  Logs,
  WiiBanners,
  Count
};

struct DirectoryEntry
{
  bool isDirectory = false;
  u64 size = 0;
  std::string physicalName;
  std::string virtualName;
  std::vector<DirectoryEntry> children;
};

class FileInfo
{
public:
  explicit FileInfo(const std::string& path);
  explicit FileInfo(const char* path);

  bool exists() const;
  bool is_directory() const;
  bool is_file() const;
  u64 size() const;

private:
  std::filesystem::file_status m_status{};
  std::uintmax_t m_size = 0;
  bool m_exists = false;
};

enum class IfAbsentBehavior
{
  ConsoleWarning,
  NoConsoleWarning
};

void initialize(std::optional<std::string> custom_root = std::nullopt);
void set_path(UserPath key, std::string path);
const std::string& get_path(UserPath key);

bool exists(std::string_view path);
bool is_directory(std::string_view path);
bool is_file(std::string_view path);

u64 file_size(std::string_view path);
u64 file_size(std::FILE* file);

bool create_dir(std::string_view path);
bool create_dirs(std::string_view path);
bool ensure_parent_dirs(std::string_view path);

bool remove_file(std::string_view path,
                 IfAbsentBehavior behavior = IfAbsentBehavior::NoConsoleWarning);
bool remove_dir(std::string_view path,
                IfAbsentBehavior behavior = IfAbsentBehavior::NoConsoleWarning);
bool remove_dir_recursive(std::string_view path);

bool rename_path(std::string_view from, std::string_view to);
bool rename_path_sync(std::string_view from, std::string_view to);
bool copy_file(std::string_view from, std::string_view to);
bool create_empty_file(std::string_view path);

DirectoryEntry scan_directory_tree(std::string path, bool recursive);

std::vector<std::string> scan_directory(
    const std::string& path, bool recursive,
    const std::function<bool(const std::filesystem::directory_entry&)>& filter = nullptr);

std::vector<std::string> glob_files(const std::vector<std::string>& roots,
                                    const std::vector<std::string>& extensions);

}  // namespace dik::fs

namespace File
{

using FSTEntry = dik::fs::DirectoryEntry;
using FileInfo = dik::fs::FileInfo;
using IfAbsentBehavior = dik::fs::IfAbsentBehavior;

inline constexpr unsigned D_USER_IDX = static_cast<unsigned>(dik::fs::UserPath::User);
inline constexpr unsigned D_WIIROOT_IDX = static_cast<unsigned>(dik::fs::UserPath::WiiRoot);
inline constexpr unsigned D_SESSION_WIIROOT_IDX =
    static_cast<unsigned>(dik::fs::UserPath::WiiSessionRoot);
inline constexpr unsigned D_CACHE_IDX = static_cast<unsigned>(dik::fs::UserPath::Cache);
inline constexpr unsigned D_REDUMPCACHE_IDX =
    static_cast<unsigned>(dik::fs::UserPath::RedumpCache);
inline constexpr unsigned D_LOGS_IDX = static_cast<unsigned>(dik::fs::UserPath::Logs);
inline constexpr unsigned D_BANNERS_WIIROOT_IDX =
    static_cast<unsigned>(dik::fs::UserPath::WiiBanners);
inline constexpr unsigned NUM_PATH_INDICES =
    static_cast<unsigned>(dik::fs::UserPath::Count);

const std::string& GetUserPath(unsigned index);
void SetUserPath(unsigned index, std::string path);

bool Exists(const std::string& path);
bool IsDirectory(const std::string& path);
bool IsFile(const std::string& path);

u64 GetSize(const std::string& path);
u64 GetSize(std::FILE* file);

bool CreateDir(const std::string& path);
bool CreateDirs(std::string_view path);
bool CreateFullPath(std::string_view full_path);

bool Delete(const std::string& path,
            IfAbsentBehavior behavior = IfAbsentBehavior::ConsoleWarning);
bool DeleteDir(const std::string& path,
               IfAbsentBehavior behavior = IfAbsentBehavior::ConsoleWarning);
bool DeleteDirRecursively(const std::string& path);

bool Rename(const std::string& from, const std::string& to);
bool RenameSync(const std::string& from, const std::string& to);
bool CopyRegularFile(std::string_view from, std::string_view to);
bool CreateEmptyFile(const std::string& path);

FSTEntry ScanDirectoryTree(std::string path, bool recursive);

}  // namespace File
