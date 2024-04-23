#pragma once

#include "glass/WindowManager.h"
#include <filesystem>
#include <memory>
#include <portable-file-dialogs.h>
#include "dbcppp/Network.h"

namespace imcan {

class DbcNetwork {
public:


private:
	std::string version;
	std::vector<std::string> new_symbols;
	std::unique_ptr<dbcppp::IBitTiming> bit_timing;
	std::vector<std::unique_ptr<dbcppp::INode>> nodes;
	std::vector<std::unique_ptr<dbcppp::IValueTable>> value_tables;
	std::vector<std::unique_ptr<dbcppp::IMessage>> messages;
	std::vector<std::unique_ptr<dbcppp::IEnvironmentVariable>> environment_variables;
	std::vector<std::unique_ptr<dbcppp::IAttributeDefinition>> attribute_definitions;
	std::vector<std::unique_ptr<dbcppp::IAttribute>> attribute_defaults;
	std::vector<std::unique_ptr<dbcppp::IAttribute>> attribute_values;
	std::string comment;

	// TODO: factory from INetwork

}; // class DbcNetwork

// TODO: in vein of DbcNetwork, add DbcNode, DbcMessage, DbcSignal, etc.
// because dbcppp thought it was nice to make everything immutable after loading. yay!

class DbcDatabase {
public:
	DbcDatabase(std::filesystem::path _filepath, std::unique_ptr<dbcppp::INetwork> _network) :
		filepath(_filepath), filename(_filepath.filename()), network(std::move(_network)) {}

	std::filesystem::path filepath;
	std::string filename;
	bool hasChanges = false;

	const std::unique_ptr<dbcppp::INetwork>& getNetwork() {
		return network;
	}

private:
	std::unique_ptr<dbcppp::INetwork> network;
}; // class DbcDatabase

class DbcManager : public glass::WindowManager {
public:
	explicit DbcManager(glass::Storage& storage) : WindowManager{storage} {};

	void DisplayLoader();

private:
	std::vector<DbcDatabase> sDatabases;
	bool addDatabase(std::filesystem::path path);
}; // class DbcManager

class DbcGui {
public:
	static void GlobalInit();

	static std::unique_ptr<DbcManager> sDbcManager;
}; // class DbcGui

} // namespace imcan