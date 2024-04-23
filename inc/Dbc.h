#pragma once

#include "glass/WindowManager.h"
#include <filesystem>
#include <memory>
#include <portable-file-dialogs.h>
#include "dbcppp/Network.h"

namespace imcan {

class DbcDatabase {
public:
	explicit DbcDatabase(std::filesystem::path _filepath, std::unique_ptr<dbcppp::INetwork> _network) :
		filepath(_filepath), filename(_filepath.filename()), network(std::move(_network)) {}

	const std::filesystem::path filepath;
	const std::string filename;
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