#include "console_system.h"
#include "console/cvar.h"
#include "gdextension_interface.h"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

ConsoleSystem *ConsoleSystem::singleton = nullptr;
CVar ConsoleSystem::save_changed_vars = CVar::create_variable("save_changed_vars", GDEXTENSION_VARIANT_TYPE_BOOL, true, "Should changed variables be saved?", PROPERTY_HINT_NONE, "");
CVar ConsoleSystem::print_changed_command = CVar::create_command("print_changed", "Prints CVars that are different from their defaults");
const char *CVAR_CONFIG_FILE_PATH = "user://cvars.cfg";
const char *CVAR_SECTION_NAME = "cvars";

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

    print_changed_command.connect_command_callback(callable_mp(this, &ConsoleSystem::print_changed_cvars));

    cvar_config_file.instantiate();
    if (FileAccess::file_exists(CVAR_CONFIG_FILE_PATH)) {
        cvar_config_file->load("user://cvars.cfg");

        if (cvar_config_file->has_section(CVAR_SECTION_NAME)) {
            for (const String &key : cvar_config_file->get_section_keys(CVAR_SECTION_NAME)) {
                set_cvar(key, cvar_config_file->get_value(CVAR_SECTION_NAME, key));
            }  
        }
    }
}

CVar *ConsoleSystem::get_cvar(StringName p_cvar_name) {
    auto it = registered_cvars.find(p_cvar_name);
    ERR_FAIL_COND_V_MSG(it == registered_cvars.end(), nullptr, vformat("CVar %s did not exist!", p_cvar_name));
    return it->value;
}

bool ConsoleSystem::set_cvar(const StringName &p_cvar, const Variant &p_value, bool p_silent, bool p_show_error) {
    auto it = registered_cvars.find(p_cvar);
    ERR_FAIL_COND_V_MSG(it == registered_cvars.end(), false, vformat("CVar %s did not exist!", p_cvar));
    ERR_FAIL_COND_V_MSG(it->value->is_command(), false, vformat("Tried to set CVar %s with a value, but it's a command!"));
    const int64_t input_type = UtilityFunctions::type_of(p_value);
    ERR_FAIL_COND_V_MSG(input_type != it->value->cvar_data->type, false, vformat("Tried to set CVar %s with Variant of type %d but expected %d!", p_cvar, input_type, it->value->cvar_data->type));

    it->value->cvar_data->current_value = p_value;

    return true;
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
                // Special case so we can support 1 as true and 0 as false, like in source.
                case GDEXTENSION_VARIANT_TYPE_BOOL: {
                    if (value.is_valid_int()) {
                        final_value = value.to_int() > 0 ? true : false;
                    }
                } break;
                default: {
                    final_value = UtilityFunctions::str_to_var(value);
                }
            }
            
            if (!set_cvar(cvar->get_cvar_name(), final_value)) {
                return;
            }
            _update_cvar_autosave(cvar);
        }
        print_line_rich(vformat("[color=red]%s = \"%s\"[/color] (def. \"%s\")", cvar->get_cvar_name_string(), cvar->get_value_display_string(), cvar->get_default_value_display_string()));

        if (value.is_empty()) {
            print_line_rich("[color=gray]- " + cvar->cvar_data->description + "[/color]");
        }
    } else {
        cvar->execute_command();
    }
}

void ConsoleSystem::print_changed_cvars() {
    print_line("Changed variables:");
    for (auto kv : registered_cvars) {
        const CVar *cvar = kv.value;
        if (cvar->is_command()) {
            continue;
        }
        const bool is_different = cvar->cvar_data->current_value != cvar->cvar_data->default_value;
        if (!is_different) {
            continue;
        }
        print_line_rich(vformat("[b]%s[/b] = [color=red]\"%s\"[/color] (def. \"%s\")", cvar->get_cvar_name_string(), cvar->get_value_display_string(), cvar->get_default_value_display_string()));
    }
}

void ConsoleSystem::_update_cvar_autosave(CVar *p_cvar) {
    if (!save_changed_vars.get_bool()) {
        return;
    }

    const StringName cvar_name = p_cvar->get_cvar_name();
    if (cvar_config_file->has_section_key(CVAR_SECTION_NAME, cvar_name)) {
        cvar_config_file->erase_section_key(CVAR_SECTION_NAME, cvar_name);
    }

    if (p_cvar->cvar_data->current_value == p_cvar->cvar_data->default_value) {
        cvar_config_file->save(CVAR_CONFIG_FILE_PATH);
        return;
    }
    
    cvar_config_file->set_value(CVAR_SECTION_NAME, cvar_name, p_cvar->cvar_data->current_value);
    cvar_config_file->save(CVAR_CONFIG_FILE_PATH);
}

ConsoleSystem::ConsoleSystem() {
    singleton = this;
}
