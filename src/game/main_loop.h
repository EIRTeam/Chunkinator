#pragma once

#include "console/console_system.h"
#include "debug/debug_overlay.h"
#include "godot_cpp/classes/scene_tree.hpp"
#include "../console/gui/console_gui.h"

using namespace godot;

class LaniakeaMainLoop : public SceneTree {
    GDCLASS(LaniakeaMainLoop, SceneTree);

    static CVar quit_command;

    ConsoleSystem *console_system;
    ConsoleGUI *console_gui;
    DebugOverlay *debug_overlay;
public:
    static void _bind_methods();
	virtual void _initialize() override;
	virtual bool _process(double p_delta) override;
	virtual void _finalize() override;
};