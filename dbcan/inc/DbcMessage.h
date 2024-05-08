#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "DbcSignal.h"
#include "ctre.hpp"

namespace dbcan {
class Message {
 public:
	uint64_t id = 0;
	std::string name = "";
	uint8_t length = 0;
	std::string transmitter = "";
	std::string comment = "";

	// TODO: toDbcString
	static std::optional<Message> fromString(std::string line);

 private:
	static constexpr auto kMsgRegexCtre =
		ctll::fixed_string { R"~(BO_\s*(\d+)\s+(\w+):\s*(\d+)\s(\w+).*)~" };
};

}  // namespace dbcan