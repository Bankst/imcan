#include "DbcMessage.h"

#include <memory>

#include "ctre.hpp"

namespace dbcan {

Message::Ptr Message::fromString(std::string line) {
	auto msg = std::make_shared<Message>();
	bool parsedOk = false;
	if (auto [whole, id, name, len, txr] = ctre::match<kMsgRegexCtre>(line); whole) {
		msg->id = std::stoull(id.to_string());
		msg->name = name;
		msg->length = std::stoi(len.to_string());
		msg->transmitter = txr;
		parsedOk = true;
	}

	if (parsedOk) { return msg; }
	return {};
}

bool Message::addSignal(Signal::Ptr sig) {
	uint64_t id = signals.size() + 1;
	sig->msgIndex = id;
	auto [iter, ok] = signals.insert({ id, sig });
	return ok;
}

bool Message::deleteSignal(uint64_t sigId) { return signals.erase(sigId) == 1; }

}  // namespace dbcan