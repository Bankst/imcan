#include <istream>
#include <optional>
#include <string_view>

#ifndef USE_CTRE
#include <regex>
#else
#include <ctre.hpp>
#endif

#include "DbcSignal.h"

namespace dbcan {

const std::regex Signal::rgx_ { Signal::kSigRegex };

std::istream& operator>>(std::istream& is, Signal& sig) {
	std::string line;
	std::getline(is, line);
	auto sigOpt = Signal::fromString(line);
	if (sigOpt) { sig = sigOpt.value(); }
	return is;
}

std::optional<Signal> Signal::fromString(std::string_view line) {
	Signal sig;
	bool parsedOk = false;
#ifndef USE_CTRE
	std::smatch match;
	std::string lineStr { line };
	if (std::regex_match(lineStr, match, Signal::rgx_)) {
		sig.name = match[1];
		bool muxOk = match[2].matched && match[2] != "";
		sig.multiplexerIndicator = muxOk ? std::make_optional(match[2]) : std::nullopt;
		sig.startBit = std::stoi(match[3]);
		sig.length = std::stoi(match[4]);
		sig.byteOrder = match[5] == '0' ? BigEndian : LittleEndian;
		sig.valueType = match[6] == '+' ? Unsigned : Signed;
		sig.scale = std::stod(match[7]);
		sig.offset = std::stod(match[8]);
		sig.valueRange = std::make_pair(std::stod(match[9]), std::stod(match[10]));
		bool unitOk = match[11].matched && match[11] != "";
		sig.unit = unitOk ? std::make_optional(match[11]) : std::nullopt;
		sig.transmitter = match[12];
		parsedOk = true;
	}
#else  // USE_CTRE
	if (auto [whole, name, mux, sbit, len, bord, vtyp, scale, offs, valmin, valmax, unit, txr] =
				ctre::match<kSigRegexCtre>(line);
			whole) {
		sig.name = name;
		sig.multiplexerIndicator = mux.to_optional_string();
		sig.startBit = std::stoi(sbit.to_string());
		sig.length = std::stoi(len.to_string());
		sig.byteOrder = bord == "0" ? BigEndian : LittleEndian;
		sig.valueType = vtyp == "+" ? Unsigned : Signed;
		sig.scale = std::stod(scale.to_string());
		sig.offset = std::stod(offs.to_string());
		sig.valueRange = std::make_pair(std::stod(valmin.to_string()), std::stod(valmax.to_string()));
		sig.unit = unit.to_optional_string();
		sig.transmitter = txr;
		parsedOk = true;
	}
#endif

	if (parsedOk) { return sig; }
	return std::nullopt;
}

std::string Signal::toPrettyString(int indentCount) const {
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
		valueRange.second, unitStr, transmitter);

	if (indentCount != 0) {
		out.insert(0, (long) indentCount * 2, ' ');
		size_t pos = out.find("\n  ");
		while (pos != std::string::npos) {
			out.insert(pos + 1, (long) indentCount * 2, ' ');
			pos = out.find("\n  ", pos + 1);
		}
	}

	return out;
}

}  // namespace dbcan