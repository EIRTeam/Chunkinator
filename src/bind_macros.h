#pragma once

#include "godot_cpp/core/defs.hpp"

#define MAKE_BIND_T(m_class, m_name, m_type, m_property_hint, m_hint_string) \
    ClassDB::bind_method(D_METHOD("set_" _STR(m_name), _STR(m_name)), &m_class::set_##m_name); \
    ClassDB::bind_method(D_METHOD("get_" _STR(m_name)), &m_class::get_##m_name); \
    ADD_PROPERTY(PropertyInfo(m_type, _STR(m_name), m_property_hint, m_hint_string), "set_" _STR(m_name), "get_" _STR(m_name));

#define MAKE_BIND_STRING_NAME(m_class, m_name) MAKE_BIND_T(m_class, m_name, Variant::STRING_NAME, PROPERTY_HINT_NONE, "");
#define MAKE_BIND_FLOAT(m_class, m_name) MAKE_BIND_T(m_class, m_name, Variant::FLOAT, PROPERTY_HINT_NONE, "");
#define MAKE_BIND_INT(m_class, m_name) MAKE_BIND_T(m_class, m_name, Variant::INT, PROPERTY_HINT_NONE, "");
#define MAKE_BIND_RESOURCE(m_class, m_name, m_resource_type) MAKE_BIND_T(m_class, m_name, Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, _STR(m_resource_type));