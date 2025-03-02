#include <fmt/format.h>

#include <iostream>
#include <string>

#include "DbcNetwork.h"

int main() {
	// Create a network from the DBC file
	auto network = dbcan::Network::createFromDBC("acura_ilx_2016_nidec.dbc");
	if (!network) {
		fmt::println(stderr, "DBC load failure!");
		return 1;
	}

	// Print all parsed data
	fmt::println("Version: {}", network->version != "" ? network->version : "No Version!");

	// Print attributes
	auto attrCount = network->attributes.size();
	fmt::println("Attributes ({}){}", attrCount, attrCount != 0 ? ":" : "");
	for (const auto& [key, value] : network->attributes) { fmt::println("  {}: {}", key, value); }

	// Print nodes
	std::cout << "Nodes:\n";
	for (const auto& node : network->nodes) { fmt::println("  {}", node); }

	// Print messages and their signals
	fmt::println("Messages: ({})", (int) network->messages.size());
	for (const auto& [id, msg] : network->messages) {
		fmt::println(
			"  Message ID: {}, Name: {}, Length: {}, Signals: {}", id, msg->name, msg->length,
			msg->signals.size());
		for (const auto& [_, signal] : msg->signals) {
			std::cout << signal->toPrettyString(2) << std::endl;
		}
	}

	return 0;
}
