#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string_view>

using namespace std::chrono;

#ifndef USE_CTRE
#include <regex>
#else
#include <ctre.hpp>
#endif

#include "DbcMessage.h"
#include "DbcNetwork.h"
#include "DbcSignal.h"

namespace dbcan {

std::optional<Network> Network::createFromDBC(const std::string &filename) {
	Network net;
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filename << std::endl;
		return std::nullopt;
	}

	// TODO: fail if insufficient tokens found

	std::string line;
	int64_t msg_counter = -1;
	auto parse_begin = steady_clock::now();
	uint32_t sig_counter = 0;
	while (std::getline(file, line)) {
		std::string_view lineView { line };

		// fmt::println("scanning - {}", line);
		bool isVersion = ctre::starts_with<"VERSION">(lineView);
		bool isNodes = ctre::starts_with<"BU_">(lineView);
		bool isMsg = ctre::starts_with<"BO_">(lineView);
		bool isSig = ctre::starts_with<" +SG_">(lineView);
		bool isAttr = ctre::starts_with<"BA_">(lineView);
#ifndef USE_CTRE
		std::smatch match;
		line = lineView;
#endif

		if (lineView.empty() || lineView[0] == '#') {
			continue;
		} else if (isVersion) {
#ifndef USE_CTRE
			std::regex versionRegex(kVersionRegex);
			auto regok = std::regex_search(line, match, versionRegex) && match.size() > 1;
			net.version = regok ? std::make_optional(match[1].str()) : std::nullopt;
#else
			if (auto [whole, version] = ctre::match<kVersionRegexCtre>(lineView); whole) {
				net.version = version;
			} else {
				net.version = std::nullopt;
			}
#endif
		} else if (isNodes) {
			std::istringstream nodesStream = {};
#ifndef USE_CTRE
			std::regex versionRegex(kNodesRegex);
			if (std::regex_search(line, match, versionRegex) && match.size() > 1) {
				nodesStream = std::istringstream { match[1].str() };
			}
#else
			if (auto [whole, nodes] = ctre::match<kNodesRegexCtre>(lineView); whole) {
				nodesStream = std::istringstream { nodes.str() };
			}
#endif
			if (!nodesStream.str().empty()) {
				std::string node;
				while (nodesStream >> node) { net.unusedNodes.push_back(node); }
			}
		} else if (isMsg) {
			if (auto msg = Message::fromString(line); msg) {
				net.messages[++msg_counter] = std::move(msg.value());
			} else {
				fmt::println("Err: failed parse msg: {}", line);
			}
		} else if (isSig && msg_counter != -1) {
			if (auto sig = Signal::fromString(line); sig) {
				net.messages[msg_counter].signals.push_back(std::move(sig.value()));
				sig_counter++;
			} else {
				fmt::println("Err: failed parse sig: {}", line);
			}
			// TODO: maybe sort signals by start bit ascending?
		} else if (isAttr) {
#ifndef USE_CTRE
// std::regex attrRegex(kAttrsRegex);
// if (std::regex_search(line, match, attrRegex) && match.size() == 5) {
// 	net.attributes[match[1].str() + match[3].str()] = match[4].str();
// }
#else
			// lol attributes are complex i dont wanna
#endif
		}
	}

	auto parse_elapsed = steady_clock::now() - parse_begin;
	auto nanos = duration_cast<nanoseconds>(parse_elapsed).count();
	fmt::println(
		"Parsed {} ({}msgs, {}sigs) in {:.3f} ms", filename, net.messages.size(), sig_counter,
		(double) nanos / 1000000.0);

	return net;
}
}  // namespace dbcan