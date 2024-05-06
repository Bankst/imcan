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
	msg = Message::fromString(line);
	return is;
}

Message Message::fromString(std::string_view line) {
	Message msg;
	std::smatch match;

#ifndef USE_CTRE
	std::string lstr { line };  // because libstd::regex sucks eggs
	if (std::regex_match(lstr, match, Message::rgx_)) {
		msg.id = std::stoull(match[1].str());
		msg.name = match[2];
		msg.length = static_cast<uint8_t>(std::stoi(match[3]));
		msg.transmitter = match[4];
	}
#else  // USE_CTRE
	ctre::match<kMsgRegexCtre>(line);
#endif
	return msg;
}

}  // namespace dbcan