// #include "DbcMessage.h"

#include <istream>
#include <regex>

#include "DbcMessage.h"

namespace dbcan {

const std::regex Message::rgx_ = std::regex(Message::kMsgRegex);

// DbcMessage
std::istream& operator>>(std::istream& is, Message& msg) {
	std::string line;
	std::getline(is, line);
	std::smatch match;
	if (std::regex_match(line, match, Message::rgx_)) {
		msg.id = std::stoull(match[1].str());
		msg.name = match[2];
		msg.length = static_cast<uint8_t>(std::stoi(match[3]));
		msg.transmitter = match[4];
	}
	return is;
}

}  // namespace dbcan