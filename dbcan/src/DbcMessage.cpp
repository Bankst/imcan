// #include "DbcMessage.h"

#include <istream>

#ifndef USE_CTRE
#include <regex>
#else
#include <ctre.hpp>
#endif

#include "DbcMessage.h"

namespace dbcan {

const std::regex Message::rgx_ = std::regex(Message::kMsgRegex);

// DbcMessage
std::istream& operator>>(std::istream& is, Message& msg) {
	std::string line;
	std::getline(is, line);
	auto msgOpt = Message::fromString(line);
	if (msgOpt) { msg = msgOpt.value(); }
	return is;
}

std::optional<Message> Message::fromString(std::string_view line) {
	Message msg;
	std::smatch match;
	bool parsedOk = false;
#ifndef USE_CTRE
	std::string lstr { line };  // because libstd::regex sucks eggs
	if (std::regex_match(lstr, match, Message::rgx_)) {
		msg.id = std::stoull(match[1].str());
		msg.name = match[2];
		msg.length = static_cast<uint8_t>(std::stoi(match[3]));
		msg.transmitter = match[4];
		parsedOk = true;
	}
#else  // USE_CTRE
	if (auto [whole, id, name, len, txr] = ctre::match<kMsgRegexCtre>(line); whole) {
		msg.id = std::stoull(id.to_string());
		msg.name = name;
		msg.length = std::stoi(len.to_string());
		msg.transmitter = txr;
		msg.signals = std::vector<Signal>(msg.length);
		parsedOk = true;
	}
#endif

	if (parsedOk) { return msg; }
	return std::nullopt;
}

}  // namespace dbcan