#pragma once

#include "glass/WindowManager.h"
#include <filesystem>
#include <memory>
#include <portable-file-dialogs.h>
// #include <CANObjects.h>

namespace imcan {

class DbcDatabase {
public:
	DbcDatabase() {};
	~DbcDatabase() {};

	std::filesystem::path filepath;
	std::string filename;
	// CppCAN::CANDatabase db;

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