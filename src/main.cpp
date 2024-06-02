#include <wpigui.h>

#include "ImCan.h"
#include "glass/Context.h"
#include "glass/MainMenuBar.h"
#include "imgui.h"

namespace gui = wpi::gui;

static glass::MainMenuBar gMainMenu;
static bool gAbout = false;
static bool gDbcMgr = false;

// for rapid iteration of signals table
void signalTableTesting() {
	static bool testShitOpen = true;
	static int flags =
		ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_RowBg;

	static std::vector<ImColor> signalColors = { 0xf210f2, 0xf9f61b, 0x6edd37,
																							 0x23d2ed, 0x07ba5d, 0x00b600 };

	std::vector<imcan::DbcMessageView::SignalBitView> bitViews = {
		{ 0, 8 }, { 8, 8 }, { 16, 8 }, { 24, 4 }, { 28, 4 }
	};

	if (ImGui::Begin("TestShit", &testShitOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
		if (ImGui::BeginTable("bits", 8, flags)) {
			for (int row = 0; row < 8; row++) {
				// auto rowBeginCurPos = ImGui::GetCursorScreenPos();
				ImGui::TableNextRow(0, 25);

				// TODO: text overlap
				// const ImColor color = ImColor(1.0f, 0.2f, 1.0f, 1.0f);
				// ImDrawList *drawList = ImGui::GetWindowDrawList();
				// drawList->AddText(rowBeginCurPos, color, "TEST");

				for (int col = 7; col >= 0; col--) {
					ImGui::TableNextColumn();
					int bit = row * 8 + col;

					ImGui::Text("%d", bit);
					ImGui::Text("Stuff");

					// todo: get signal from bitview
					imcan::DbcMessageView::SignalBitView *thisBv = nullptr;
					int bvNum = 0;
					for (auto &bv : bitViews) {
						if (bv.bitInside(bit)) {
							thisBv = &bv;
							break;
						}
						bvNum++;
					}

					if (thisBv != nullptr) {
						auto color = signalColors[bvNum];
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, color);
					}
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

#ifdef _WIN32
int __stdcall WinMain(void *hInstance, void *hPrevInstance, char *pCmdLine, int nCmdShow) {
	int argc = __argc;
	char **argv = __argv;
#else
auto main(int argc, char **argv) -> int {
#endif
	std::string_view saveDir;
	if (argc == 2) { saveDir = argv[1]; }

	gui::CreateContext();
	glass::CreateContext();

	gui::AddEarlyExecute([] { ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()); });
	gui::AddInit([] { ImGui::GetIO().ConfigDockingWithShift = true; });

	glass::SetStorageName("imcan");
	glass::SetStorageDir(saveDir.empty() ? gui::GetPlatformSaveFileDir() : saveDir);

	gui::AddInit([] { glass::ResetTime(); });

	gui::AddLateExecute([] { gMainMenu.Display(); });

	bool showDemo = false;

	gMainMenu.AddMainMenu([&] {
		if (ImGui::BeginMenu("Windows")) {
			bool dbcMgrVis = imcan::DbcGui::sDbcManager->GetWindow("Loader")->IsVisible();
			std::string dbcMgrItem = dbcMgrVis ? "DBC Loader (Hide)" : "DBC Loader (Show)";
			if (ImGui::MenuItem(dbcMgrItem.c_str())) {
				imcan::DbcGui::sDbcManager->GetWindow("Loader")->SetVisible(!dbcMgrVis);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Info")) {
			if (ImGui::MenuItem("About")) { gAbout = true; }
			ImGui::EndMenu();
		}
		ImGui::Checkbox("Demo", &showDemo);
	});

	gui::AddLateExecute([&] {
		if (gAbout) {
			ImGui::OpenPopup("About");
			gAbout = false;
		}
		if (ImGui::BeginPopupModal("About")) {
			ImGui::Text("ImCan: A modern CAN workstation");
			ImGui::Separator();
			ImGui::Text("Save location: %s", glass::GetStorageDir().c_str());
			ImGui::Text(
				"%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}

		if (showDemo) { ImGui::ShowDemoWindow(&showDemo); }
		signalTableTesting();
	});

	gui::Initialize("ImCan", 1024, 768, ImGuiConfigFlags_DockingEnable);
	imcan::DbcGui::GlobalInit();
	gui::Main();

	glass::DestroyContext();
	gui::DestroyContext();

	return 0;
}