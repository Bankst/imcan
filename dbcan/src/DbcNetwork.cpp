#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string_view>

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
	Message curMsg = {};
	while (std::getline(file, line)) {
		std::string_view lineView { line };

		auto pos = lineView.find_first_not_of(' ');
		lineView = lineView.substr(pos != std::string::npos ? pos : 0);

#ifndef USE_CTRE
		std::smatch match;
		line = lineView;
#endif

		if (lineView.empty() || lineView[0] == '#') {
			continue;
		} else if (lineView.find("VERSION ") == 0) {
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
		} else if (lineView.find("BU_:") == 0) {
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
		} else if (line.find("BO_ ") == 0) {
			if (curMsg.id != 0) {
				net.messages[curMsg.id] = curMsg;
				curMsg.signals.clear();
			}
			curMsg = Message::fromString(line);
		} else if (line.find(" SG_ ") == 0 && curMsg.id != 0) {
			Signal sig = Signal::fromString(line);
			// TODO: maybe sort signals by start bit ascending?
			curMsg.signals.push_back(sig);
		} else if (line.find("BA_") == 0) {
#ifndef USE_CTRE
// std::regex attrRegex(kAttrsRegex);
// if (std::regex_search(line, match, attrRegex) && match.size() == 5) {
// 	net.attributes[match[1].str() + match[3].str()] = match[4].str();
// }
#else
			// if (auto [whole, ])
#endif
		}

		if (curMsg.id != 0) { net.messages[curMsg.id] = curMsg; }
	}

	return net;
}
}  // namespace dbcan