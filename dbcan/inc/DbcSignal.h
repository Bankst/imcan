#pragma once

#include <fmt/format.h>

#include <memory>
#include <string>

#include "ctre.hpp"

namespace dbcan {

enum ByteOrder : uint8_t { BigEndian = 0, LittleEndian = 1 };
enum ValueType : char { Unsigned, Signed };

class Signal {
	// TODO: split `muxData` to enum (none, switch, val) and int

 public:
	using Ptr = std::shared_ptr<Signal>;

	/*
	 * 1-indexed.
	 * This is NOT part of the DBC.
	 * Only used for identifying signals in a message for deletion.
	 */
	uint64_t msgIndex = 0;

	std::string name = "";            // Required
	std::string muxData;              // Optional. multiplexer indicator.
	int startBit = 0;                 // Required
	int length = 0;                   // Required
	ByteOrder byteOrder = BigEndian;  // Required. `0` for Big-endian, `1` for Little-endian
	ValueType valueType = Unsigned;   // Required. `+` for unsigned, `-` for signed
	double scale = 1;                 // Required
	double offset = 0;                // Required
	std::pair<double, double> valueRange { 0, 1 };  // Required
	std::string unit = "";                          // Optional
	std::string transmitter = "";                   // Required
	std::string comment = "";                       // Optional

	// TODO: toDbcString
	[[nodiscard]] std::string toPrettyString(int indentCount = 0) const;

	static Signal::Ptr fromString(std::string line);

 private:
	static constexpr auto kSigRegexCtre = ctll::fixed_string {
		R"~( +SG_\s(\w+)\s*(M|m\d+)?\s*:\s*(\d+)\|(\d+)@([01])([\+\-])\s*\(([^,]+),([^)]+)\)\s*\[(-?\d+(?:\.?\d+)*)\|(-?\d+(?:\.?\d+)*)\]\s*['"]([^'"]*)['"]\s+(\w+))~"
	};
};

}  // namespace dbcan