#pragma once

#include <portable-file-dialogs.h>

#include <filesystem>
#include <memory>
#include <string>

#include "Attribute.h"
#include "BitTiming.h"
#include "Message.h"
#include "dbcppp/Network.h"
#include "glass/WindowManager.h"

namespace imcan {

using namespace dbcppp;

class DbcNetwork {
 public:
 private:
	std::string version;
	std::vector<std::string> new_symbols;
	std::unique_ptr<dbcppp::IBitTiming> bit_timing;
	std::vector<std::unique_ptr<INode>> nodes;
	std::vector<std::unique_ptr<IValueTable>> value_tables;
	std::vector<std::unique_ptr<IMessage>> messages;
	std::vector<std::unique_ptr<IEnvironmentVariable>> environment_variables;
	std::vector<std::unique_ptr<IAttributeDefinition>> attribute_definitions;
	std::vector<std::unique_ptr<IAttribute>> attribute_defaults;
	std::vector<std::unique_ptr<IAttribute>> attribute_values;
	std::string comment;

	static std::vector<std::string> buildNewSymbols(INetwork &net) {
		return { net.NewSymbols().begin(), net.NewSymbols().end() };
	}

	static std::vector<std::unique_ptr<INode>> buildNodes(INetwork &net) {
		std::vector<std::unique_ptr<INode>> nodes;
		for (auto &node : net.Nodes()) { nodes.push_back(node.Clone()); }
		return nodes;
	}

	static std::vector<std::unique_ptr<INode>> buildValueTables(INetwork &net) {
		std::vector<std::unique_ptr<INode>> valuetables;
		for (auto &node : net.Nodes()) { valuetables.push_back(node.Clone()); }
		return valuetables;
	}

 public:
	// TODO: factory from INetwork
	DbcNetwork(dbcppp::INetwork &network) {
		version = network.Version();
		for (auto symbol : network.NewSymbols()) { new_symbols.push_back(symbol); }
		bit_timing = network.BitTiming().Clone();
		for (auto &node : network.Nodes()) { nodes.push_back(std::move(node.Clone())); }
		for (auto &value_table : network.ValueTables()) {
			value_tables.push_back(std::move(value_table.Clone()));
		}
		for (auto &message : network.Messages()) { messages.push_back(std::move(message.Clone())); }
		for (auto &envvar : network.EnvironmentVariables()) {
			environment_variables.push_back(std::move(envvar.Clone()));
		}
		for (auto &attrdef : network.AttributeDefinitions()) {
			attribute_definitions.push_back(std::move(attrdef.Clone()));
		}
		for (auto &attrdefault : network.AttributeDefaults()) {
			attribute_defaults.push_back(std::move(attrdefault.Clone()));
		}
		for (auto &attrval : network.AttributeValues()) {
			attribute_values.push_back(std::move(attrval.Clone()));
		}
		comment = network.Comment();
		network.Merge(nullptr);
	}

	static void AddMessage(INetwork &net, IMessage &msg) {
		// make new empty network just to stuff a message in the old one
		std::vector<std::unique_ptr<IMessage>> messages;
		messages.push_back(msg.Clone());
		// auto newNet = INetwork::Create(
		// 	"", std::move(buildNewSymbols(net)), net.BitTiming().Clone(), std::move(net.Nodes(),
		// net.ValueTables(), 	messages, net.EnvironmentVariables(), net.AttributeDefinitions(),
		// net.AttributeDefaults(), 	net.AttributeValues(), net.Comment());
	}
};  // class DbcNetwork

using DbcMessageMap = std::unordered_map<uint64_t, const dbcppp::IMessage *>;

class DbcMessage {};

class DbcSignal {
 public:
	DbcSignal(
		uint64_t message_size, std::string &&name, ISignal::EMultiplexer multiplexer_indicator,
		uint64_t multiplexer_switch_value, uint64_t start_bit, uint64_t bit_size,
		ISignal::EByteOrder byte_order, ISignal::EValueType value_type, double factor, double offset,
		double minimum, double maximum, std::string &&unit, std::vector<std::string> &&receivers) {}

 private:
	std::string _name;
	ISignal::EMultiplexer _multiplexer_indicator;
	uint64_t _multiplexer_switch_value;
	uint64_t _start_bit;
	uint64_t _bit_size;
	ISignal::EByteOrder _byte_order;
	ISignal::EValueType _value_type;
	double _factor;
	double _offset;
	double _minimum;
	double _maximum;
	std::string _unit;
	std::vector<std::string> _receivers;
	std::string _comment;

	/* Saving this stuff for later
	std::vector<IAttribute> _attribute_values;
	std::vector<IValueEncodingDescription> _value_encoding_descriptions;
	ISignal::EExtendedValueType _extended_value_type;
	std::vector<ISignalMultiplexerValue> _signal_multiplexer_values;
	*/
};
// TODO: in vein of DbcNetwork, add DbcNode, DbcMessage, DbcSignal, etc.
// because dbcppp thought it was nice to make everything immutable after
// loading. yay!

class DbcDatabase {
 public:
	DbcDatabase(std::filesystem::path _filepath, std::unique_ptr<dbcppp::INetwork> _network)
			: filepath(_filepath), filename(_filepath.filename()), network(std::move(_network)) {}

	std::filesystem::path filepath;
	std::string filename;
	bool hasChanges = false;

	const std::unique_ptr<dbcppp::INetwork> &getNetwork() { return network; }

 private:
	std::unique_ptr<dbcppp::INetwork> network;
};  // class DbcDatabase

class DbcManager : public glass::WindowManager {
 public:
	explicit DbcManager(glass::Storage &storage) : WindowManager { storage } {};

	void DisplayLoader();

 private:
	std::vector<DbcDatabase> sDatabases;
	bool addDatabase(std::filesystem::path path);
};  // class DbcManager

class DbcGui {
 public:
	static void GlobalInit();

	static std::unique_ptr<DbcManager> sDbcManager;
};  // class DbcGui

}  // namespace imcan