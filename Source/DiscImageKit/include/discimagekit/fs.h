#pragma once

#include <string>

namespace dik
{
// Returns the user directory path used by DiscImageKit after initialization.
const std::string& user_directory();

// Initializes directories needed by DiscImageKit. If custom_path is empty, a directory under the
// system temporary directory will be used.
void initialize_user_directory(const std::string& custom_path);
}
