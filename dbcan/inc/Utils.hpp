#pragma once

#include <algorithm>
#include <string>

namespace utils {

// Function to trim leading whitespace
std::string ltrim(const std::string &s) {
	auto start =
		std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
	return { start, s.end() };
}

}  // namespace utils