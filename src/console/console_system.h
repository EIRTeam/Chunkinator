#pragma once

#include "console/cvar.h"
#include "godot_cpp/templates/hash_map.hpp"
#include "godot_cpp/templates/local_vector.hpp"
#include <vector>

using namespace godot;

class ConsoleSystem {
    HashMap<StringName, CVar*> registered_cvars; 
    HashMap<StringName, Ref<CVarProxy>> cvar_proxies; 
    static ConsoleSystem *singleton;

    void register_cvar(CVar* p_cvar);
    Ref<CVarProxy> get_proxy(const StringName &p_cvar) const;
public:
    static ConsoleSystem *get_singleton();
    void initialize();
    
    CVar *get_cvar(StringName p_cvar_name);

    struct CVarAutocompleteResult {
        CVar *cvar;
        float similarity;
    };

    Vector<CVarAutocompleteResult> do_autocomplete(const String &p_text) const;
    void execute_user_command(const String &p_command_line);

    ConsoleSystem();
    friend class CVar;
};