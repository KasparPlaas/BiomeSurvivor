"""
SetupTestLevel.py - Unreal Engine Python Editor Script
Run from the UE5 Editor: Tools > Execute Python Script > select this file

Creates a basic test level with:
- Ground plane (10x10 meter cube, scaled flat)
- PlayerStart at origin
- SkyAtmosphere + SkyLight for environment
- ExponentialHeightFog
- DayNightCycle and WeatherSystem are auto-spawned by GameMode

After running, save the level as /Game/Maps/TestLevel
"""

import unreal

def setup_test_level():
    editor = unreal.EditorLevelLibrary
    world = unreal.EditorLevelLibrary.get_editor_world()
    
    if not world:
        unreal.log_error("No editor world found!")
        return
    
    # ----- Ground Plane -----
    # Spawn a large cube mesh as ground
    ground_loc = unreal.Vector(0, 0, -50)
    ground_rot = unreal.Rotator(0, 0, 0)
    ground = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor, ground_loc, ground_rot
    )
    if ground:
        ground.set_actor_label("Ground")
        ground.set_actor_scale3d(unreal.Vector(100, 100, 1))
        mesh_comp = ground.static_mesh_component
        # Use engine's basic cube mesh
        cube_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
        if cube_mesh:
            mesh_comp.set_static_mesh(cube_mesh)
        # Apply a basic material
        basic_mat = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/BasicShapeMaterial")
        if basic_mat:
            mesh_comp.set_material(0, basic_mat)
        unreal.log("Created ground plane")
    
    # ----- Player Start -----
    ps_loc = unreal.Vector(0, 0, 100)
    ps_rot = unreal.Rotator(0, 0, 0)
    player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart, ps_loc, ps_rot
    )
    if player_start:
        player_start.set_actor_label("PlayerStart_Test")
        unreal.log("Created PlayerStart")
    
    # ----- Sky Atmosphere -----
    sky_atmo = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.load_class(None, "/Script/Engine.SkyAtmosphere"),
        unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0)
    )
    if sky_atmo:
        sky_atmo.set_actor_label("SkyAtmosphere")
        unreal.log("Created SkyAtmosphere")
    
    # ----- Sky Light -----
    sky_light_class = unreal.load_class(None, "/Script/Engine.SkyLight")
    if sky_light_class:
        sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
            sky_light_class,
            unreal.Vector(0, 0, 500), unreal.Rotator(0, 0, 0)
        )
        if sky_light:
            sky_light.set_actor_label("SkyLight")
            unreal.log("Created SkyLight")
    
    # ----- Exponential Height Fog -----
    fog_class = unreal.load_class(None, "/Script/Engine.ExponentialHeightFog")
    if fog_class:
        fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
            fog_class,
            unreal.Vector(0, 0, 200), unreal.Rotator(0, 0, 0)
        )
        if fog:
            fog.set_actor_label("ExponentialHeightFog")
            unreal.log("Created ExponentialHeightFog")
    
    unreal.log("========================================")
    unreal.log("Test level setup complete!")
    unreal.log("Save this level as: /Game/Maps/TestLevel")
    unreal.log("DayNightCycle and WeatherSystem will auto-spawn from GameMode.")
    unreal.log("Hit Play to test!")
    unreal.log("========================================")

# Run
setup_test_level()
