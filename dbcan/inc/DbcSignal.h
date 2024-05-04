#pragma once

#include <fmt/format.h>

#include <iostream>
#include <optional>
#include <regex>
#include <string>

namespace dbcan {

enum ByteOrder : uint8_t { BigEndian = 0, LittleEndian = 1 };
enum ValueType : char { Unsigned, Signed };

class Signal {
 public:
	inline static int s_sigCounter = 0;

	std::string name;
	// todo: split to enum (none, switch, val) and int
	std::optional<std::string> multiplexerIndicator;  // Optional multiplexer indicator
	int startBit;
	int length;
	ByteOrder byteOrder;  // '0' for Big-endian, '1' for Little-endian
	ValueType valueType;  // '+' for unsigned, '-' for signed
	double scale;
	double offset;
	std::pair<int, int> valueRange;  // Signal value range
	std::optional<std::string> unit;
	std::string node;

	friend std::istream& operator>>(std::istream& is, Signal& sig);

	std::string toString(int indentCount = 0) const;

 private:
	static constexpr auto kSigRegex =
		R"~(\sSG_\s(\w+)\s*(M|m\d+)?\s*:\s*(\d+)\|(\d+)@([01])([+-])\s*\(([^,]+),([^)]+)\)\s*\[(-?\d+)\|(-?\d+)\]\s*['"]([^'"]*)['"]\s+(\w+))~";
	static const std::regex rgx_;
};

}  // namespace dbcan