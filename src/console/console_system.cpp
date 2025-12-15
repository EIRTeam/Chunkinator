#include "console_system.h"
#include "console/cvar.h"
#include "gdextension_interface.h"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include <cstdio>

ConsoleSystem *ConsoleSystem::singleton = nullptr;

void ConsoleSystem::register_cvar(CVar* p_cvar) {
    auto it = registered_cvars.find(p_cvar->cvar_data->cvar_name);
    print_line("REGISTERING CVAR!", p_cvar->get_cvar_name_string());
    if (it != registered_cvars.end()) {
        ERR_FAIL_MSG(vformat("Tried to register CVar %s, but it already exists!", p_cvar->cvar_data->cvar_name));
        return;
    }
    registered_cvars.insert(p_cvar->cvar_data->cvar_name, p_cvar);
    
    Ref<CVarProxy> proxy;
    proxy.instantiate();
    cvar_proxies.insert(p_cvar->cvar_data->cvar_name, proxy);
}

Ref<CVarProxy> ConsoleSystem::get_proxy(const StringName &p_cvar) const {
    ERR_FAIL_COND_V_MSG(!cvar_proxies.has(p_cvar), nullptr, "Invalid CVar");
    return cvar_proxies[p_cvar];
}

ConsoleSystem *ConsoleSystem::get_singleton() {
    return singleton;
}

void ConsoleSystem::initialize() {
    std::vector<CVar::DelayedInitData> delayed_init_arr = CVar::get_delayed_init_arr();
    for (int i = 0; i < delayed_init_arr.size(); i++) {
        delayed_init_arr[i].cvar->_delayed_init(delayed_init_arr[i]);
    }
}

CVar *ConsoleSystem::get_cvar(StringName p_cvar_name) {
    auto it = registered_cvars.find(p_cvar_name);
    ERR_FAIL_COND_V_MSG(it == registered_cvars.end(), nullptr, vformat("CVar %s did not exist!", p_cvar_name));
    return it->value;
}

Vector<ConsoleSystem::CVarAutocompleteResult> ConsoleSystem::do_autocomplete(const String &p_text) const {
    Vector<ConsoleSystem::CVarAutocompleteResult> result;
    for (auto &pair : registered_cvars) {
        const String &cvar_name = pair.value->cvar_data->cvar_name_str;
        float similarity = cvar_name.similarity(p_text);
        if (similarity < 0.1f) {
            continue;
        }
        if (cvar_name.begins_with(p_text)) {
            similarity *= 2.0f;
        }
        result.push_back({
            .cvar = pair.value,
            .similarity = similarity
        });
    }

    struct _AutocompleteComparator {
	    _FORCE_INLINE_ bool operator()(const ConsoleSystem::CVarAutocompleteResult &a, const ConsoleSystem::CVarAutocompleteResult &b) const { return (a.similarity >= b.similarity); }
    };

    result.sort_custom<_AutocompleteComparator>();

    return result;
}

void ConsoleSystem::execute_user_command(const String &p_command_line) {
    const String command_line_stripped = p_command_line.strip_edges();

    if (command_line_stripped.is_empty()) {
        print_line("]");
        return;
    }

    const int64_t first_space = command_line_stripped.find(" ");
    print_line("] " + command_line_stripped);
    
    StringName cvar_name = command_line_stripped.substr(0, first_space);

    auto it = registered_cvars.find(cvar_name);

    if (it == registered_cvars.end()) {
        print_error(vformat("CVar \"%s\" not found!", cvar_name));
        return;
    }

    CVar *cvar = it->value;

    if (!cvar->is_command()) {
        const String value = first_space != -1 && (first_space+1) < command_line_stripped.length() ? command_line_stripped.substr(first_space+1) : "";

        if (!value.is_empty()) {
            Variant final_value = cvar->cvar_data->current_value;

            switch(cvar->cvar_data->type) {
                case GDEXTENSION_VARIANT_TYPE_BOOL: {
                    if (value.is_valid_int()) {
                        final_value = value.to_int() > 0 ? true : false;
                    }
                } break;
                default: {
                    final_value = UtilityFunctions::str_to_var(value);
                }
            }
            
            if (UtilityFunctions::type_of(final_value) != cvar->cvar_data->type) {
                print_error("Invalid value!");
                return;
            }

            cvar->cvar_data->current_value = final_value;
        }
        print_line_rich(vformat("[color=red]%s = \"%s\"[/color] (def. \"%s\")", cvar->get_cvar_name_string(), cvar->get_value_display_string(), cvar->get_default_value_display_string()));

        if (value.is_empty()) {
            print_line_rich("[color=gray]- " + cvar->cvar_data->description + "[/color]");
        }
    } else {
        cvar->execute_command();
    }
}

ConsoleSystem::ConsoleSystem() {
    singleton = this;
}
