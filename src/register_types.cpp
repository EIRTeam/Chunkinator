#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "chunkinator/chunkinator_test.h"
#include "example_class.h"
#include "quadtree.h"
#include "worldman.h"
#include "debugger/worldman_debugger.h"
#include "debugger/superchunk_map.h"
#include "debugger/superchunk_debugger.h"

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	GDREGISTER_CLASS(ExampleClass);
	GDREGISTER_CLASS(QuadTree);
	GDREGISTER_CLASS(WorldMan);
	GDREGISTER_CLASS(WorldManDebugger);
	GDREGISTER_CLASS(ChunkinatorTest);
	GDREGISTER_ABSTRACT_CLASS(SuperchunkMap);
	GDREGISTER_ABSTRACT_CLASS(SuperchunkDebugger);
	GDREGISTER_ABSTRACT_CLASS(Chunkinator);
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C"
{
	// Initialization
	GDExtensionBool GDE_EXPORT chunkinator_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_gdextension_types);
		init_obj.register_terminator(uninitialize_gdextension_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}