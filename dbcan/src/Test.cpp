#include <fmt/base.h>

#include <iostream>
#include <string>

#include "DbcNetwork.h"

int main2() {
	std::istringstream signalData(
		"SG_ INS_Vel_Sideways_2D M : 40|24@1- (0.0001,0) [-838|838] \"m/s\" Vector__XXX\n"
		"SG_ SensorSelect m0 : 0|8@1+ (1,0) [0|100] '' InstrumentCluster\n"
		"SG_ SensorTime : 8|16@1+ (1,0) [0|65535] '' InstrumentCluster\n"
		"SG_ Sensor1Value : 24|16@1+ (1,0) [0|65535] 'g' InstrumentCluster");

	dbcan::Signal sig;
	while (signalData >> sig) { std::cout << sig.toString() << std::endl; }

	return 0;
}

int main() {
	// Create a network from the DBC file
	std::optional<dbcan::Network> networkOpt = dbcan::Network::createFromDBC("vehicle.dbc");
	if (!networkOpt) {
		fmt::println(stderr, "DBC load failure!");
		return 1;
	}
	dbcan::Network network = networkOpt.value();

	// Print all parsed data
	fmt::println("Version: {}", network.version.has_value() ? network.version.value() : "N/A");

	// Print attributes
	int attrCount = network.attributes.size();
	fmt::println("Attributes ({}){}", attrCount, attrCount != 0 ? ":" : "");
	for (const auto& [key, value] : network.attributes) { fmt::println("  {}: {}", key, value); }

	// Print unused nodes
	std::cout << "Unused Nodes:\n";
	for (const auto& node : network.unusedNodes) { fmt::println("  {}", node); }

	// Print messages and their signals
	std::cout << "Messages:\n";
	for (const auto& [id, msg] : network.messages) {
		fmt::println(
			"  Message ID: {}, Name: {}, Length: {}, Signals: {}", id, msg.name, msg.length,
			msg.signals.size());
		for (const auto& signal : msg.signals) { std::cout << signal.toString(2) << std::endl; }
	}

	return 0;
}
