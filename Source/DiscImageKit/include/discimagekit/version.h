#pragma once

#include <string>

namespace dik::version
{
inline const std::string& scm_description()
{
  static const std::string value = "discimagekit";
  return value;
}

inline const std::string& scm_revision()
{
  static const std::string value = "unknown";
  return value;
}

inline const std::string& user_agent()
{
  static const std::string value = "discimagekit";
  return value;
}
}

