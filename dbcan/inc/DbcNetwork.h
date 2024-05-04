#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "DbcMessage.h"

namespace dbcan {

class Network {
 public:
	std::optional<std::string> version;
	std::unordered_map<std::string, std::string> attributes;
	std::vector<std::string> unusedNodes;
	std::map<uint64_t, Message> messages;

	static std::optional<Network> createFromDBC(const std::string& filename);
};

}  // namespace dbcan