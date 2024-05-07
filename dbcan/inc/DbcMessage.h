#pragma once

#include <cstdint>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include "DbcSignal.h"

#ifdef USE_CTRE
#include "ctre.hpp"
#endif

namespace dbcan {

class Message {
 public:
	uint64_t id;
	std::string name;
	uint8_t length;
	std::string transmitter;

	std::vector<Signal> signals;

	friend std::istream& operator>>(std::istream& is, Message& msg);

	// TODO: toDbcString
	static std::optional<Message> fromString(std::string_view line);

 private:
	static constexpr auto kMsgRegex = R"(BO_\s*(\d+)\s+(\w+):\s*(\d+)\s(\w+).*)";
#ifdef USE_CTRE
	static constexpr auto kMsgRegexCtre =
		ctll::fixed_string { R"~(BO_\s*(\d+)\s+(\w+):\s*(\d+)\s(\w+).*)~" };
#endif

	static const std::regex rgx_;
};

}  // namespace dbcan