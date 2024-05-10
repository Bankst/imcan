#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "DbcMessage.h"
#include "ctre.hpp"

namespace dbcan {

class Network {
 public:
	std::string version = "";
	std::vector<std::string> infoBlock;
	std::unordered_map<std::string, std::string> attributes;
	std::vector<std::string> nodes;
	std::map<uint64_t, std::shared_ptr<Message>> messages;

	// TODO: some way to track diff of changes?
	bool hasChanges = false;

	bool deleteMessage(uint64_t msgId);

	static std::shared_ptr<Network> createFromDBC(const std::string& filename);
	// TODO: toDbcString

 private:
	static constexpr auto kVersionRegexCtre = ctll::fixed_string { R"~(VERSION "([^\"]+)")~" };
	static constexpr auto kNodesRegexCtre = ctll::fixed_string { R"~(BU_: ([^;]+))~" };
	static constexpr auto kAttrsRegexCtre =
		ctll::fixed_string { R"~(BA_ "(\w+)" (BO_|BU_|SG_) (\d+) ("[^"]+");)~" };
};

}  // namespace dbcan