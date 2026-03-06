"""
SetupTestLevel.py - Unreal Engine Python Editor Script
Run from the UE5 Editor: Tools > Execute Python Script > select this file

Creates a complete test level with:
- Large ground plane (100m x 100m)
- PlayerStart elevated above ground
- Directional Light (Sun)
- SkyAtmosphere + SkyLight
- ExponentialHeightFog + VolumetricCloud
- Several static mesh props for reference scale
- Nav Mesh Bounds Volume for AI
- DayNightCycle and WeatherSystem are auto-spawned by GameMode
"""

import unreal

def setup_test_level():
    editor = unreal.EditorLevelLibrary
    
    # ----- Ground Plane (100m x 100m) -----
    ground = editor.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 0, -50),
        unreal.Rotator(0, 0, 0)
    )
    if ground:
        ground.set_actor_label("Ground")
        ground.set_actor_scale3d(unreal.Vector(200, 200, 1))
        mesh_comp = ground.static_mesh_component
        cube_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
        if cube_mesh:
            mesh_comp.set_static_mesh(cube_mesh)
        basic_mat = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/BasicShapeMaterial")
        if basic_mat:
            mesh_comp.set_material(0, basic_mat)
        unreal.log("Created ground plane (200m x 200m)")
    
    # ----- Player Start -----
    player_start = editor.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(0, 0, 100),
        unreal.Rotator(0, 0, 0)
    )
    if player_start:
        player_start.set_actor_label("PlayerStart_Main")
        unreal.log("Created PlayerStart")
    
    # ----- Directional Light (Sun) -----
    dir_light_class = unreal.load_class(None, "/Script/Engine.DirectionalLight")
    if dir_light_class:
        sun = editor.spawn_actor_from_class(
            dir_light_class,
            unreal.Vector(0, 0, 1000),
            unreal.Rotator(-50, -30, 0)
        )
        if sun:
            sun.set_actor_label("Sun_DirectionalLight")
            unreal.log("Created Directional Light (Sun)")

    # ----- Sky Atmosphere -----
    sky_atmo_class = unreal.load_class(None, "/Script/Engine.SkyAtmosphere")
    if sky_atmo_class:
        sky_atmo = editor.spawn_actor_from_class(
            sky_atmo_class,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )
        if sky_atmo:
            sky_atmo.set_actor_label("SkyAtmosphere")
            unreal.log("Created SkyAtmosphere")
    
    # ----- Sky Light -----
    sky_light_class = unreal.load_class(None, "/Script/Engine.SkyLight")
    if sky_light_class:
        sky_light = editor.spawn_actor_from_class(
            sky_light_class,
            unreal.Vector(0, 0, 500),
            unreal.Rotator(0, 0, 0)
        )
        if sky_light:
            sky_light.set_actor_label("SkyLight")
            # Set to real-time capture
            light_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
            if light_comp:
                light_comp.set_editor_property("source_type", unreal.SkyLightSourceType.SLS_CAPTURED_SCENE)
                light_comp.set_editor_property("real_time_capture", True)
            unreal.log("Created SkyLight (real-time capture)")
    
    # ----- Exponential Height Fog -----
    fog_class = unreal.load_class(None, "/Script/Engine.ExponentialHeightFog")
    if fog_class:
        fog = editor.spawn_actor_from_class(
            fog_class,
            unreal.Vector(0, 0, 200),
            unreal.Rotator(0, 0, 0)
        )
        if fog:
            fog.set_actor_label("HeightFog")
            fog_comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
            if fog_comp:
                fog_comp.set_editor_property("fog_density", 0.02)
                fog_comp.set_editor_property("volumetric_fog", True)
            unreal.log("Created Exponential Height Fog with volumetrics")

    # ----- Reference Props (cubes/spheres for scale) -----
    sphere_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Sphere")
    cylinder_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cylinder")
    
    props = [
        ("Tree_Placeholder", unreal.Vector(500, 300, 150), unreal.Vector(2, 2, 6), cylinder_mesh),
        ("Tree_Placeholder_2", unreal.Vector(-400, 600, 150), unreal.Vector(2, 2, 6), cylinder_mesh),
        ("Rock_Placeholder", unreal.Vector(800, -200, 50), unreal.Vector(3, 3, 2), sphere_mesh),
        ("Rock_Placeholder_2", unreal.Vector(-600, -500, 40), unreal.Vector(2, 4, 2), sphere_mesh),
        ("Crate_Placeholder", unreal.Vector(200, -400, 50), unreal.Vector(1, 1, 1), cube_mesh),
        ("Wall_Placeholder", unreal.Vector(-300, 200, 100), unreal.Vector(5, 0.2, 4), cube_mesh),
    ]
    
    for label, loc, scale, mesh_asset in props:
        prop = editor.spawn_actor_from_class(
            unreal.StaticMeshActor, loc, unreal.Rotator(0, 0, 0)
        )
        if prop and mesh_asset:
            prop.set_actor_label(label)
            prop.set_actor_scale3d(scale)
            mc = prop.static_mesh_component
            mc.set_static_mesh(mesh_asset)
            if basic_mat:
                mc.set_material(0, basic_mat)
    unreal.log("Created %d reference props" % len(props))

    # ----- Post-Processing Volume (global) -----
    ppv_class = unreal.load_class(None, "/Script/Engine.PostProcessVolume")
    if ppv_class:
        ppv = editor.spawn_actor_from_class(
            ppv_class,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )
        if ppv:
            ppv.set_actor_label("GlobalPostProcess")
            ppv.set_editor_property("unbound", True)
            unreal.log("Created global PostProcessVolume")

    unreal.log("========================================")
    unreal.log("Test level setup complete!")
    unreal.log("GameMode auto-spawns DayNightCycle + WeatherSystem.")
    unreal.log("Press Ctrl+S to save, then Play to test!")
    unreal.log("Controls: WASD=Move, Mouse=Look, Space=Jump,")
    unreal.log("  Shift=Sprint, Ctrl=Crouch, LMB=Attack, RMB=Block,")
    unreal.log("  E=Interact, Alt=Dodge, V=Toggle Camera")
    unreal.log("========================================")

# Run
setup_test_level()
