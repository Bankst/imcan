#pragma once

#include <fmt/format.h>

#include <optional>
#include <string>

#include "ctre.hpp"

namespace dbcan {

enum ByteOrder : uint8_t { BigEndian = 0, LittleEndian = 1 };
enum ValueType : char { Unsigned, Signed };

class Signal {
 public:
	std::string name;
	// todo: split to enum (none, switch, val) and int
	std::optional<std::string> muxData;  // Optional multiplexer indicator
	int startBit;
	int length;
	ByteOrder byteOrder;  // '0' for Big-endian, '1' for Little-endian
	ValueType valueType;  // '+' for unsigned, '-' for signed
	double scale;
	double offset;
	std::pair<double, double> valueRange;  // Signal value range
	std::optional<std::string> unit;
	std::string transmitter;

	// TODO: toDbcString
	std::string toPrettyString(int indentCount = 0) const;

	static std::optional<Signal> fromString(std::string line);

 private:
	static constexpr auto kSigRegexCtre = ctll::fixed_string {
		R"~( +SG_\s(\w+)\s*(M|m\d+)?\s*:\s*(\d+)\|(\d+)@([01])([\+\-])\s*\(([^,]+),([^)]+)\)\s*\[(-?\d+(?:\.?\d+)*)\|(-?\d+(?:\.?\d+)*)\]\s*['"]([^'"]*)['"]\s+(\w+))~"
	};
};

}  // namespace dbcan