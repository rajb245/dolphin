#pragma once

#include <algorithm>
#include <iterator>
#include <string_view>
#include <vector>

#include "discimagekit/types.h"

namespace dik::algorithms
{

template <typename Range, typename T>
bool contains(const Range& range, const T& value)
{
  return std::find(std::begin(range), std::end(range), value) != std::end(range);
}

inline bool contains_subrange(const std::vector<u8>& haystack, std::string_view needle)
{
  if (needle.empty())
    return true;
  return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end()) !=
         haystack.end();
}

}  // namespace dik::algorithms

