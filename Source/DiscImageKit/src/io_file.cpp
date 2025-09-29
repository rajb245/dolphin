#include "discimagekit/io_file.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace File
{
namespace
{
long ConvertOrigin(SeekOrigin origin)
{
  switch (origin)
  {
  case SeekOrigin::Begin:
    return SEEK_SET;
  case SeekOrigin::Current:
    return SEEK_CUR;
  case SeekOrigin::End:
    return SEEK_END;
  }
  return SEEK_SET;
}
}

IOFile::IOFile() : m_file(nullptr), m_good(true)
{
}

IOFile::IOFile(std::FILE* file) : m_file(file), m_good(file != nullptr)
{
}

IOFile::IOFile(const std::string& filename, const char openmode[], SharedAccess)
    : m_file(nullptr), m_good(true)
{
  Open(filename, openmode);
}

IOFile::~IOFile()
{
  Close();
}

IOFile::IOFile(IOFile&& other) noexcept : m_file(nullptr), m_good(true)
{
  Swap(other);
}

IOFile& IOFile::operator=(IOFile&& other) noexcept
{
  if (this != &other)
    Swap(other);
  return *this;
}

void IOFile::Swap(IOFile& other) noexcept
{
  std::swap(m_file, other.m_file);
  std::swap(m_good, other.m_good);
}

bool IOFile::Open(const std::string& filename, const char openmode[], SharedAccess)
{
  Close();
  m_file = std::fopen(filename.c_str(), openmode);
  m_good = m_file != nullptr;
  return m_good;
}

bool IOFile::Close()
{
  if (m_file)
  {
    if (std::fclose(m_file) != 0)
      m_good = false;
    m_file = nullptr;
  }
  return m_good;
}

IOFile IOFile::Duplicate(const char openmode[]) const
{
  if (!IsOpen())
    return IOFile();
#ifdef _WIN32
  int fd = _fileno(m_file);
  if (fd == -1)
    return IOFile();
  FILE* dup_handle = _fdopen(_dup(fd), openmode);
  return IOFile(dup_handle);
#else
  int fd = fileno(m_file);
  if (fd == -1)
    return IOFile();
  int dup_fd = dup(fd);
  if (dup_fd == -1)
    return IOFile();
  FILE* dup_handle = fdopen(dup_fd, openmode);
  if (!dup_handle)
  {
    close(dup_fd);
    return IOFile();
  }
  return IOFile(dup_handle);
#endif
}

bool IOFile::Seek(s64 offset, SeekOrigin origin)
{
  if (!IsOpen())
  {
    m_good = false;
    return false;
  }

#ifdef _WIN32
  if (_fseeki64(m_file, offset, ConvertOrigin(origin)) != 0)
#else
  if (fseeko(m_file, offset, ConvertOrigin(origin)) != 0)
#endif
    m_good = false;

  return m_good;
}

u64 IOFile::Tell() const
{
  if (!IsOpen())
    return 0;
#ifdef _WIN32
  return static_cast<u64>(_ftelli64(m_file));
#else
  return static_cast<u64>(ftello(m_file));
#endif
}

u64 IOFile::GetSize() const
{
  if (!IsOpen())
    return 0;
  const auto current = Tell();
  const_cast<IOFile*>(this)->Seek(0, SeekOrigin::End);
  const u64 size = Tell();
  const_cast<IOFile*>(this)->Seek(static_cast<s64>(current), SeekOrigin::Begin);
  return size;
}

bool IOFile::Resize(u64 size)
{
  if (!IsOpen())
  {
    m_good = false;
    return false;
  }
#ifdef _WIN32
  m_good = _chsize_s(_fileno(m_file), size) == 0;
#else
  m_good = ftruncate(fileno(m_file), size) == 0;
#endif
  return m_good;
}

bool IOFile::Flush()
{
  if (!IsOpen())
  {
    m_good = false;
    return false;
  }
  if (std::fflush(m_file) != 0)
    m_good = false;
  return m_good;
}

void IOFile::SetHandle(std::FILE* file)
{
  Close();
  ClearError();
  m_file = file;
}

}  // namespace File
