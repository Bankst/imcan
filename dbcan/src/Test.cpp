#include <fmt/format.h>

#include <iostream>
#include <string>

#include "DbcNetwork.h"

int main() {
	// Create a network from the DBC file
	auto network = dbcan::Network::createFromDBC("vehicle.dbc");
	if (!network) {
		fmt::println(stderr, "DBC load failure!");
		return 1;
	}

	// Print all parsed data
	fmt::println("Version: {}", network->version);

	// Print attributes
	auto attrCount = network->attributes.size();
	fmt::println("Attributes ({}){}", attrCount, attrCount != 0 ? ":" : "");
	for (const auto& [key, value] : network->attributes) { fmt::println("  {}: {}", key, value); }

	// Print unused nodes
	std::cout << "Unused Nodes:\n";
	// for (const auto& node : network->unusedNodes) { fmt::println("  {}", node); }

	// Print messages and their signals
	fmt::println("Messages: ({})", (int) network->messages.size());
	return 0;
	for (const auto& [id, msg] : network->messages) {
		fmt::println(
			"  Message ID: {}, Name: {}, Length: {}, Signals: {}", id, msg.name, msg.length,
			msg.signals.size());
		for (const auto& signal : msg.signals) { std::cout << signal.toPrettyString(2) << std::endl; }
	}

	return 0;
}
