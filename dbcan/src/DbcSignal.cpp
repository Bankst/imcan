#include <istream>
#include <optional>
#include <regex>

#include "DbcSignal.h"

namespace dbcan {

const std::regex Signal::rgx_ { Signal::kSigRegex };

std::istream& operator>>(std::istream& is, Signal& sig) {
	std::string line;
	std::getline(is, line);
	std::smatch match;
	if (std::regex_match(line, match, Signal::rgx_)) {
		sig.name = match[1];
		bool muxOk = match[2].matched && match[2] != "";
		sig.multiplexerIndicator = muxOk ? std::make_optional(match[2]) : std::nullopt;
		sig.startBit = std::stoi(match[3]);
		sig.length = std::stoi(match[4]);
		sig.byteOrder = match[5] == '0' ? BigEndian : LittleEndian;
		sig.valueType = match[6] == '+' ? Unsigned : Signed;
		sig.scale = std::stod(match[7]);
		sig.offset = std::stod(match[8]);
		sig.valueRange = std::make_pair(std::stoi(match[9]), std::stoi(match[10]));
		bool unitOk = match[11].matched && match[11] != "";
		sig.unit = unitOk ? std::make_optional(match[11]) : std::nullopt;
		sig.node = match[12];

		fmt::println("Parsed Signal #{} ({})", ++Signal::s_sigCounter, sig.name);
	}
	return is;
}

std::string Signal::toString(int indentCount) const {
	std::string mux = multiplexerIndicator.has_value() ? multiplexerIndicator.value() : "N/A";
	std::string unitStr = unit.has_value() ? unit.value() : "N/A";
	std::string byteOrderStr = byteOrder == BigEndian ? "BigEndian" : "LittleEndian";
	std::string valueTypeStr = valueType == Unsigned ? "Unsigned" : "Signed";

	std::string out = fmt::format(
		"Signal Name: {}"
		"\n  Multiplexer Indicator: {}"
		"\n  Start Bit: {}"
		"\n  Length: {}"
		"\n  Byte Order: {}"
		"\n  Value Type: {}"
		"\n  Scale: {}"
		"\n  Offset: {}"
		"\n  Value Range: [{}, {}]"
		"\n  Unit: {}"
		"\n  Transmitter: {}",
		name, mux, startBit, length, byteOrderStr, valueTypeStr, scale, offset, valueRange.first,
		valueRange.second, unitStr, node);

	if (indentCount != 0) {
		out.insert(0, indentCount * 2, ' ');
		size_t pos = out.find("\n  ");
		while (pos != std::string::npos) {
			out.insert(pos + 1, indentCount * 2, ' ');
			pos = out.find("\n  ", pos + 1);
		}
	}

	return out;
}

}  // namespace dbcan