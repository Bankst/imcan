#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Utils.hpp"

using namespace std::chrono;

#include <ctre.hpp>

#include "DbcMessage.h"
#include "DbcNetwork.h"
#include "DbcSignal.h"

namespace dbcan {

std::shared_ptr<Network> Network::createFromDBC(const std::string &filename) {
	auto net = std::make_shared<Network>();  // todo: is this best?

	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filename << std::endl;
		return nullptr;
	}

	// TODO: fail if insufficient tokens found

	std::string line;
	std::optional<uint64_t> curMsgId = std::nullopt;
	auto parse_begin = steady_clock::now();
	uint32_t sig_counter = 0;
	int infoCounter = 0;
	while (std::getline(file, line)) {
		// fmt::println("scanning - {}", line);
		bool isVersion = ctre::starts_with<"VERSION\\s">(line);
		bool isInfoBlock = ctre::starts_with<"NS_\\s">(line);
		bool isNodes = ctre::starts_with<"BU_\\s">(line);
		bool isMsg = ctre::starts_with<"BO_\\s">(line);
		bool isSig = ctre::starts_with<" +SG_\\s">(line);
		bool isAttr = ctre::starts_with<"BA_\\s">(line);
		if (line.empty() || line[0] == '#') {
			continue;
		} else if (isVersion) {
			if (auto [whole, version] = ctre::match<kVersionRegexCtre>(line); whole) {
				net->version = version;
			}
		} else if (isInfoBlock) {
			// grab each line until newline
			while (std::getline(file, line) && !line.empty()) {
				auto data = utils::ltrim(line);
				net->infoBlock.emplace_back(data);
				infoCounter++;
			}
		} else if (isNodes) {
			std::istringstream nodesStream = {};
			if (auto [whole, nodes] = ctre::match<kNodesRegexCtre>(line); whole) {
				nodesStream = std::istringstream { nodes.str() };
				std::string node;
				while (nodesStream >> node) { net->nodes.push_back(node); }
			}
		} else if (isMsg) {
			if (auto msgPtr = Message::fromString(line); msgPtr) {
				curMsgId = msgPtr->id;
				net->messages.insert({ curMsgId.value(), std::move(msgPtr) });
			} else {
				fmt::println("Err: failed parse msg: {}", line);
			}
		} else if (isSig && curMsgId.has_value()) {  // TODO: better?
			if (auto sigPtr = Signal::fromString(line); sigPtr) {
				net->messages.at(curMsgId.value())->addSignal(sigPtr);
				sig_counter++;
			} else {
				fmt::println("Err: failed parse sig: {}", line);
			}
			// TODO: maybe sort signals by start bit ascending?
		} else if (isAttr) {
			// lol attributes are complex i dont wanna
		}
	}

	auto parse_elapsed = steady_clock::now() - parse_begin;
	auto nanos = duration_cast<nanoseconds>(parse_elapsed).count();
	fmt::println(
		"Parsed {} ({}msgs, {}sigs) in {:.3f} ms", filename, net->messages.size(), sig_counter,
		(double) nanos / 1000000.0);

	return net;
}

bool Network::deleteMessage(uint64_t msgId) { return messages.erase(msgId) == 1; }

}  // namespace dbcan