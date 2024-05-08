#include "DbcMessage.h"

#include "ctre.hpp"

namespace dbcan {

std::optional<Message> Message::fromString(std::string line) {
	Message msg;
	bool parsedOk = false;
	if (auto [whole, id, name, len, txr] = ctre::match<kMsgRegexCtre>(line); whole) {
		msg.id = std::stoull(id.to_string());
		msg.name = name;
		msg.length = std::stoi(len.to_string());
		msg.transmitter = txr;
		parsedOk = true;
	}

	if (parsedOk) { return msg; }
	return std::nullopt;
}

}  // namespace dbcan