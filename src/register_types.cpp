#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "chunkinator/chunk_spawner.h"
#include "chunkinator/chunkinator_debug_snapshot_viewer.h"
#include "chunkinator/chunkinator_debugger.h"
#include "chunkinator/chunkinator_test.h"
#include "console/console_system.h"
#include "console/cvar.h"
#include "console/gui/console_gui.h"
#include "console/gui/console_logger.h"
#include "example_class.h"
#include "game/main_loop.h"
#include "game/movement_settings.h"
#include "indirect_mesh.h"
#include "inertialization_skeleton_modifier_polynomial.h"
#include "quadtree.h"
#include "segment_quadtree.h"
#include "terrain_generator/terrain_heightmap_combine_layer.h"
#include "terrain_generator/terrain_manager.h"
#include "terrain_generator/terrain_settings.h"
#include "indirect_mesh_instance_3d.h"

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	GDREGISTER_CLASS(IndirectMesh);
	GDREGISTER_CLASS(IndirectMeshInstance3D);
	GDREGISTER_CLASS(TerrainScattererLODMesh);
	GDREGISTER_CLASS(TerrainScattererElementSettings);
	GDREGISTER_CLASS(TerrainScatterLayerSettings);
	GDREGISTER_CLASS(ExampleClass);
	GDREGISTER_CLASS(QuadTree);
	GDREGISTER_CLASS(ChunkinatorTest);
	GDREGISTER_CLASS(TerrainSettings);
	GDREGISTER_CLASS(TerrainHeightNoiseLayerSettings);
	GDREGISTER_ABSTRACT_CLASS(CVarProxy);
	GDREGISTER_ABSTRACT_CLASS(ConsoleGUI);
	GDREGISTER_CLASS(LaniakeaMainLoop);
	GDREGISTER_CLASS(MovementSettings);
	GDREGISTER_ABSTRACT_CLASS(Chunkinator);
	GDREGISTER_ABSTRACT_CLASS(ChunkinatorDebugLayerViewer);
	GDREGISTER_ABSTRACT_CLASS(ChunkinatorDebugger);
	GDREGISTER_ABSTRACT_CLASS(ChunkSpawner);
	GDREGISTER_ABSTRACT_CLASS(TerrainManager);
	GDREGISTER_ABSTRACT_CLASS(SegmentQuadTreeDebug);
	GDREGISTER_ABSTRACT_CLASS(ConsoleLogger);
	GDREGISTER_ABSTRACT_CLASS(ConsoleSystem);
	
	GDREGISTER_CLASS(InertializationSkeletonModifierPolynomial);
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