#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "DbcSignal.h"
#include "ctre.hpp"

namespace dbcan {
class Message {
 public:
	using Ptr = std::shared_ptr<Message>;

	uint64_t id = 0;
	std::string name = "";
	uint8_t length = 0;
	std::string transmitter = "";
	std::map<uint64_t, Signal::Ptr> signals;
	std::string comment = "";

	// TODO: toDbcString
	static Message::Ptr fromString(std::string line);

	bool addSignal(Signal::Ptr sig);
	bool deleteSignal(uint64_t sigId);

 private:
	static constexpr auto kMsgRegexCtre =
		ctll::fixed_string { R"~(BO_\s*(\d+)\s+(\w+):\s*(\d+)\s(\w+).*)~" };
};

}  // namespace dbcan