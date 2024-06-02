#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <optional>

#include "DbcNetwork.h"
#include "glass/WindowManager.h"

namespace imcan {

class DbcNetworkView {
 public:
	DbcNetworkView(const std::shared_ptr<dbcan::Network> net_) : m_net(net_) {}

	void Display();
	void MadeChanges() { m_net->hasChanges = true; }
	void DeleteMessage(uint64_t id);

 private:
	const std::shared_ptr<dbcan::Network> m_net;
	std::optional<uint64_t> m_msgToDelete = std::nullopt;
};

class DbcMessageView {
 public:
	DbcMessageView(DbcNetworkView *parentNet_, dbcan::Message::Ptr msg_);

	class EditCtx {
	 public:
		EditCtx(bool _modified, dbcan::Message::Ptr _msg) : modified(_modified), msg(std::move(_msg)) {}
		~EditCtx() = default;

		bool modified;
		dbcan::Message::Ptr msg;
	};
	using EditCtxPtr = std::shared_ptr<EditCtx>;

	void Display();
	void DeleteSignal(uint64_t sigId);

	bool IsEditing() const;
	void DisplayEditor();

	bool HasUnsavedChanges() const;

 private:
	void DisplayEditorInternal();

	DbcNetworkView *m_net;
	dbcan::Message::Ptr m_msg;

	const std::string m_longTitle;
	const std::string m_delModalStr;

	EditCtxPtr m_editContext;
	const std::string m_editTitle;

	void UpdateEditingMsg();
	bool BeginEdit();
	void EndEdit(bool save, bool end = true);

	uint64_t m_sigToDelete = 0;  // signals are 1-indexed, so 0 means none

	// TODO: move these to... not here? Some kind of per-DBC store.
	// Also clear them upon un-load of DBC
	static std::map<uint64_t, bool> sm_modifiedStatuses;
	static std::map<uint64_t, EditCtxPtr> sm_editingMsgs;
};

// TODO: split message editing to fully separate class
// class DbcMessageEditorContext {
//  public:
// 	DbcMessageEditorContext(const std::string title_, dbcan::Message::Ptr msg_);

//  private:
// 	const std::string m_title;
// 	dbcan::Message::Ptr m_origMsg;
// 	dbcan::Message::Ptr m_editMsg;
// 	bool modified;
// };

class DbcSignalView {
 public:
	DbcSignalView(DbcMessageView *parentMsg_, dbcan::Signal::Ptr sig_)
			: m_msg(parentMsg_), m_sig(std::move(sig_)) {}

	void Display();

 private:
	DbcMessageView *m_msg;
	dbcan::Signal::Ptr m_sig;

	void DisplayCtxMenu();
	void DisplayEditor();
};

class DbcDatabase {
 public:
	DbcDatabase(std::filesystem::path _filepath, std::shared_ptr<dbcan::Network> _network)
			: filepath(_filepath), filename(_filepath.filename()) {
		m_network = std::move(_network);
	}

	std::filesystem::path filepath;
	std::string filename;
	bool open = true;

	const std::shared_ptr<dbcan::Network> getNetwork() { return m_network; }

 private:
	std::shared_ptr<dbcan::Network> m_network;
};  // class DbcDatabase

class DbcManager : public glass::WindowManager {
	using databases_t = std::vector<DbcDatabase>;

 public:
	explicit DbcManager(glass::Storage &storage) : WindowManager { storage } {};

	void DisplayLoader();
	DbcDatabase *getCurDb();

 private:
	bool addDatabase(std::filesystem::path path);
	DbcDatabase *m_curDb;
	databases_t m_sDatabases;
};  // class DbcManager

class DbcGui {
 public:
	static void GlobalInit();

	static std::unique_ptr<DbcManager> sDbcManager;

	static void DisplayLoader();
	static void DisplayDatabase(DbcDatabase &db);
};  // class DbcGui

}  // namespace imcan