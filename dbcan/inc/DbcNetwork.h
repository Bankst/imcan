#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "DbcMessage.h"
#include "ctre.hpp"

namespace dbcan {

class Network {
 public:
	std::optional<std::string_view> version;
	std::unordered_map<std::string_view, std::string_view> attributes;
	std::vector<std::string_view> unusedNodes;
	std::map<uint64_t, Message> messages;

	static std::optional<Network> createFromDBC(const std::string& filename);
	// TODO: toDbcString

 private:
	static constexpr auto kVersionRegex = R"~(VERSION "([^\"]+)")~";
#ifdef USE_CTRE
	static constexpr auto kVersionRegexCtre = ctll::fixed_string { R"~(VERSION "([^\"]+)")~" };
#endif

	static constexpr auto kNodesRegex = R"~(BU_: ([^;]+))~";
#ifdef USE_CTRE
	static constexpr auto kNodesRegexCtre = ctll::fixed_string { R"~(BU_: ([^;]+))~" };
#endif

	static constexpr auto kAttrsRegex = R"~(BA_ "(\w+)" (BO_|BU_|SG_) (\d+) ("[^"]+");)~";
#ifdef USE_CTRE
	static constexpr auto kAttrsRegexCtre =
		ctll::fixed_string { R"~(BA_ "(\w+)" (BO_|BU_|SG_) (\d+) ("[^"]+");)~" };
#endif
};

}  // namespace dbcan