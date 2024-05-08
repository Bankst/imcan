#include "DbcSignal.h"

#include "ctre.hpp"

namespace dbcan {

Signal::Ptr Signal::fromString(std::string line) {
	auto sig = std::make_shared<Signal>();
	bool parsedOk = false;
	if (auto [whole, name, mux, sbit, len, bord, vtyp, scale, offs, valmin, valmax, unit, txr] =
				ctre::match<kSigRegexCtre>(line);
			whole) {
		sig->name = name;
		sig->muxData = mux.to_string();
		sig->startBit = std::stoi(sbit.to_string());
		sig->length = std::stoi(len.to_string());
		sig->byteOrder = bord == "0" ? BigEndian : LittleEndian;
		sig->valueType = vtyp == "+" ? Unsigned : Signed;
		sig->scale = std::stod(scale.to_string());
		sig->offset = std::stod(offs.to_string());
		sig->valueRange = std::make_pair(std::stod(valmin.to_string()), std::stod(valmax.to_string()));
		sig->unit = unit;
		sig->transmitter = txr;
		parsedOk = true;
	}

	if (parsedOk) { return sig; }
	return {};
}

std::string Signal::toPrettyString(int indentCount) const {
	std::string mux = muxData != "" ? muxData : "N/A";
	std::string unitStr = !unit.empty() ? unit : "N/A";
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