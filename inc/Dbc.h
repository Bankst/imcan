#pragma once

#include <portable-file-dialogs.h>

#include <filesystem>
#include <memory>
#include <string>

#include "DbcNetwork.h"
#include "glass/WindowManager.h"

namespace imcan {

class DbcDatabase {
 public:
	DbcDatabase(std::filesystem::path _filepath, std::shared_ptr<dbcan::Network> _network)
			: filepath(_filepath), filename(_filepath.filename()) {
		network = std::move(_network);
	}

	std::filesystem::path filepath;
	std::string filename;
	bool hasChanges = false;

	const std::shared_ptr<dbcan::Network> getNetwork() { return network; }

 private:
	std::shared_ptr<dbcan::Network> network;
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