#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>

namespace utils {

// Function to trim leading whitespace
std::string ltrim(const std::string& s) {
	auto start =
		std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
	return { start, s.end() };
}

int stoi(const std::string& str, int* p_value, std::size_t* pos = 0, int base = 10) {
	// wrapping std::stoi because it may throw an exception

	try {
		*p_value = std::stoi(str, pos, base);
		return 0;
	}

	catch (const std::invalid_argument& ia) {
		// std::cerr << "Invalid argument: " << ia.what() << std::endl;
		return -1;
	}

	catch (const std::out_of_range& oor) {
		// std::cerr << "Out of Range error: " << oor.what() << std::endl;
		return -2;
	}

	catch (const std::exception& e) {
		// std::cerr << "Undefined error: " << e.what() << std::endl;
		return -3;
	}
}

}  // namespace utils