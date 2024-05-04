#include <fstream>
#include <iostream>
#include <optional>
#include <regex>

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
		std::istringstream iss(line);
		std::smatch match;

		if (line.empty() || line[0] == '#') {
			continue;
		} else if (line.find("VERSION ") == 0) {
			std::regex versionRegex("VERSION \"([^\"]+)\"");
			if (std::regex_search(line, match, versionRegex) && match.size() > 1) {
				net.version = match[1].str();
			} else {
				net.version = std::nullopt;
			}
		} else if (line.find("BU_:") == 0) {
			std::regex nodesRegex("BU_: ([^;]+)");
			if (std::regex_search(line, match, nodesRegex) && match.size() > 1) {
				std::istringstream nodesStream(match[1].str());
				std::string node;
				while (nodesStream >> node) { net.unusedNodes.push_back(node); }
			}
		} else if (line.find("BO_ ") == 0) {
			if (curMsg.id != 0) {
				net.messages[curMsg.id] = curMsg;
				curMsg.signals.clear();
			}
			iss >> curMsg;
		} else if (line.find(" SG_ ") == 0 && curMsg.id != 0) {
			Signal sig;
			iss >> sig;
			// TODO: maybe sort signals by start bit ascending?
			curMsg.signals.push_back(sig);
		} else if (line.find("BA_") == 0) {
			std::regex attrRegex("BA_ \"(\\w+)\" (BO_|BU_|SG_) (\\d+) (\"[^\"]+\");");
			if (std::regex_search(line, match, attrRegex) && match.size() == 5) {
				net.attributes[match[1].str() + match[3].str()] = match[4].str();
			}
		}

		if (curMsg.id != 0) { net.messages[curMsg.id] = curMsg; }
	}

	return net;
}
}  // namespace dbcan