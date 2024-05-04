#pragma once

#include <cstdint>
#include <regex>
#include <string>
#include <vector>

#include "DbcSignal.h"

namespace dbcan {

class Message {
 public:
	uint64_t id;
	std::string name;
	uint8_t length;
	std::string transmitter;

	std::vector<Signal> signals;

	friend std::istream& operator>>(std::istream& is, Message& msg);

 private:
	static constexpr auto kMsgRegex = R"(BO_\s*(\d+)\s+(\w+):\s*(\d+)\s(\w+).*)";
	static const std::regex rgx_;
};

}  // namespace dbcan