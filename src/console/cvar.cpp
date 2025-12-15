#include "cvar.h"
#include "console/console_system.h"
#include "gdextension_interface.h"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

std::vector<CVar::DelayedInitData> &CVar::get_delayed_init_arr() {
    static std::vector<DelayedInitData> delayed_init;
    return delayed_init;
}

void CVar::_delayed_init(const DelayedInitData &p_delayed_init) {
    Variant default_value;

    if (!p_delayed_init.flags.has_flag(FLAG_IS_COMMAND)) {
        std::visit([&default_value](const auto &p_arg) {
            default_value = p_arg;
        }, p_delayed_init.default_value_raw);

        DEV_ASSERT(UtilityFunctions::type_of(default_value) == p_delayed_init.type);
    }

    cvar_data = memnew(CVarData);
    *cvar_data = {
        .cvar_name = StringName(p_delayed_init.cvar_name_cstr),
        .cvar_name_str = p_delayed_init.cvar_name_cstr,
        .type = p_delayed_init.type,
        .default_value = default_value,
        .description = p_delayed_init.description_cstr,
        .property_hint = p_delayed_init.property_hint,
        .property_hint_text = p_delayed_init.property_hint_text_cstr,
        .flags = p_delayed_init.flags,
        .current_value = default_value
    };
    ConsoleSystem::get_singleton()->register_cvar(this);
}

CVar::CVar(const DelayedInitData &p_delayed_init) {
    std::vector<DelayedInitData> &delayed_init_arr = get_delayed_init_arr();
    delayed_init_arr.push_back(p_delayed_init);
    delayed_init_arr[delayed_init_arr.size()-1].cvar = this;
}

CVar::~CVar() {
    if (cvar_data != nullptr) {
        memdelete(cvar_data);
    }
}

CVar CVar::create_variable(const char *p_name, int p_type, DefaultValueRaw p_default, const char *p_description, int p_property_hint, const char *p_property_hint_text) {
    return CVar({
        .cvar_name_cstr = p_name,
        .type = p_type,
        .default_value_raw = p_default,
        .description_cstr = p_description,
        .property_hint = PROPERTY_HINT_NONE,
        .property_hint_text_cstr = p_property_hint_text,
        .flags = 0
    });
}

CVar CVar::create_command(const char *p_name, const char *p_description) {
    return CVar({
        .cvar_name_cstr = p_name,
        .type = GDEXTENSION_VARIANT_TYPE_NIL,
        .default_value_raw = 0,
        .description_cstr = p_description,
        .property_hint = PROPERTY_HINT_NONE,
        .property_hint_text_cstr = "",
        .flags = FLAG_IS_COMMAND
    });
}

String CVar::get_cvar_name_string() const {
    return cvar_data->cvar_name_str;
}

StringName CVar::get_cvar_name() const {
    return cvar_data->cvar_name;
}

String CVar::_get_value_display_string(const Variant &p_variant) const {
    switch (cvar_data->type) {
        case GDEXTENSION_VARIANT_TYPE_BOOL: {
            return p_variant.booleanize() ? "1" : "0";
        } break;
    };

    return UtilityFunctions::var_to_str(cvar_data->current_value);
}

String CVar::get_value_display_string() const {
    return _get_value_display_string(cvar_data->current_value);
}

String CVar::get_default_value_display_string() const {
    return _get_value_display_string(cvar_data->default_value);
}

bool CVar::is_command() const {
    return cvar_data->flags.has_flag(FLAG_IS_COMMAND);
}

void CVar::execute_command() {
    DEV_ASSERT(is_command());
    ConsoleSystem::get_singleton()->get_proxy(cvar_data->cvar_name)->emit_signal("command_executed");
}

void CVar::connect_command_callback(Callable p_callable) {
    ConsoleSystem::get_singleton()->get_proxy(cvar_data->cvar_name)->connect("command_executed", p_callable);
}


void CVarProxy::_bind_methods() {
    ADD_SIGNAL(MethodInfo("command_executed"));
}

