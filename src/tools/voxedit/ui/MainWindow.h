/**
 * @file
 */

#pragma once

#include "ui/Window.h"
#include "core/Common.h"
#include "core/String.h"

class EditorScene;
namespace ui {
class UIApp;
}

/**
 * @brief Voxel editing tools panel
 */
class MainWindow: public ui::Window {
private:
	EditorScene* _scene;

	tb::TBCheckBox *_showGrid = nullptr;
	tb::TBCheckBox *_showAABB = nullptr;
	tb::TBCheckBox *_showAxis = nullptr;

	bool handleClickEvent(const tb::TBWidgetEvent &ev);
	bool handleChangeEvent(const tb::TBWidgetEvent &ev);
public:
	MainWindow(ui::UIApp* tool);

	bool init();

	bool OnEvent(const tb::TBWidgetEvent &ev) override;

	bool save(std::string_view file);
	bool load(std::string_view file);
	bool createNew(bool force);

};
