#include "Common/CPUDetect.h"

#include <sstream>

CPUInfo::CPUInfo()
{
  Detect();
}

std::string CPUInfo::Summarize()
{
  std::ostringstream ss;
  ss << model_name << ',' << cpu_id;
  if (bAES)
    ss << ",aes";
  if (bSHA1)
    ss << ",sha1";
  if (bSHA2)
    ss << ",sha2";
  if (bCRC32)
    ss << ",crc32";
  return ss.str();
}

void CPUInfo::Detect()
{
  vendor = CPUVendor::Other;
  model_name = "DiscImageKit CPU";
  cpu_id = "generic";
  num_cores = 1;
  bAES = true;
  bSHA1 = true;
  bSHA2 = true;
  bCRC32 = true;
}

CPUInfo cpu_info;
