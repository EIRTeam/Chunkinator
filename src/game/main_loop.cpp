#include "main_loop.h"
#include "debug/debug_overlay.h"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/core/print_string.hpp"

CVar LaniakeaMainLoop::quit_command = CVar::create_command("quit", "Quits the game");

void LaniakeaMainLoop::_bind_methods() {
    
}

void LaniakeaMainLoop::_initialize() {
    console_system = memnew(ConsoleSystem);
    console_system->initialize();

    console_gui = memnew(ConsoleGUI);
    get_root()->add_child(console_gui);
    print_line_rich("[b]Project Laniakea[/b]");
    Dictionary version_info = Engine::get_singleton()->get_version_info();
    version_info["hash"] = String(version_info["hash"]).substr(0, 9);
    print_line(String("Shinobu Engine v{major}.{minor}.{status}.{build}.{hash} - https://eirteam.moe").format(version_info));

    quit_command.connect_command_callback(callable_mp(static_cast<SceneTree*>(this), &SceneTree::quit).bind(0));

    debug_overlay = memnew(DebugOverlay);
    debug_overlay->initialize(this);
}

bool LaniakeaMainLoop::_process(double p_delta) {
    debug_overlay->advance();
    return SceneTree::_process(p_delta);
}

void LaniakeaMainLoop::_finalize() {
    memdelete(console_system);
    memdelete(debug_overlay);
}
