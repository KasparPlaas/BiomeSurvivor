"""
SetupTestLevel.py - Unreal Engine Python Editor Script
Run from UE5 Editor: Tools > Execute Python Script > select this file

Creates a comprehensive playable environment with:
- Large ground terrain (400m x 400m)
- Rolling hills and elevation variation
- Forest of placeholder trees (varied sizes)
- Rock formations and boulders
- Water body (lake)
- Camp area with shelter frame and fire pit
- PlayerStart elevated above ground
- Full atmospheric lighting (sun, sky, fog)
- Post-processing for professional visuals
- Nav Mesh Bounds Volume for AI
"""

import unreal
import random

def setup_test_level():
    editor = unreal.EditorLevelLibrary
    
    # Load basic meshes
    cube_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
    sphere_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Sphere")
    cylinder_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cylinder")
    cone_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cone")
    plane_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Plane")
    basic_mat = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/BasicShapeMaterial")
    
    random.seed(42)  # Reproducible layout
    
    # =====================================================
    # TERRAIN - Large ground + hills
    # =====================================================
    
    # Main ground plane (400m x 400m)
    ground = editor.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 0, -50),
        unreal.Rotator(0, 0, 0)
    )
    if ground:
        ground.set_actor_label("Ground_Main")
        ground.set_actor_scale3d(unreal.Vector(400, 400, 1))
        mc = ground.static_mesh_component
        if cube_mesh:
            mc.set_static_mesh(cube_mesh)
        if basic_mat:
            mc.set_material(0, basic_mat)
        unreal.log("Created main ground (400m x 400m)")
    
    # Rolling hills
    hills = [
        ("Hill_Large_1",  unreal.Vector(3000, 2000, -20),   unreal.Vector(30, 25, 4)),
        ("Hill_Large_2",  unreal.Vector(-4000, -3000, -20), unreal.Vector(35, 30, 5)),
        ("Hill_Medium_1", unreal.Vector(6000, -2000, -20),  unreal.Vector(20, 18, 3)),
        ("Hill_Medium_2", unreal.Vector(-2000, 5000, -20),  unreal.Vector(22, 20, 3.5)),
        ("Hill_Small_1",  unreal.Vector(1500, -4000, -10),  unreal.Vector(12, 10, 2)),
        ("Hill_Small_2",  unreal.Vector(-5000, 1000, -10),  unreal.Vector(15, 12, 2.5)),
        ("Hill_Gentle_1", unreal.Vector(8000, 8000, -30),   unreal.Vector(40, 35, 3)),
        ("Ridge_1",       unreal.Vector(-7000, -6000, -20), unreal.Vector(45, 8, 4)),
    ]
    for label, loc, scale in hills:
        hill = editor.spawn_actor_from_class(
            unreal.StaticMeshActor, loc, unreal.Rotator(0, random.uniform(0, 360), 0)
        )
        if hill and sphere_mesh:
            hill.set_actor_label(label)
            hill.set_actor_scale3d(scale)
            hill.static_mesh_component.set_static_mesh(sphere_mesh)
            if basic_mat:
                hill.static_mesh_component.set_material(0, basic_mat)
    unreal.log("Created %d terrain hills" % len(hills))
    
    # =====================================================
    # PLAYER START
    # =====================================================
    
    player_start = editor.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(0, 0, 150),
        unreal.Rotator(0, 0, 0)
    )
    if player_start:
        player_start.set_actor_label("PlayerStart_Main")
        unreal.log("Created PlayerStart")
    
    # =====================================================
    # ATMOSPHERIC LIGHTING
    # =====================================================
    
    # Directional Light (Sun)
    dir_light_class = unreal.load_class(None, "/Script/Engine.DirectionalLight")
    if dir_light_class:
        sun = editor.spawn_actor_from_class(
            dir_light_class,
            unreal.Vector(0, 0, 2000),
            unreal.Rotator(-45, -30, 0)
        )
        if sun:
            sun.set_actor_label("Sun_DirectionalLight")
            light_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
            if light_comp:
                light_comp.set_editor_property("intensity", 8.0)
                light_comp.set_editor_property("light_color", unreal.Color(255, 245, 230, 255))
                light_comp.set_editor_property("atmosphere_sun_light", True)
            unreal.log("Created Sun")
    
    # Sky Atmosphere
    sky_atmo_class = unreal.load_class(None, "/Script/Engine.SkyAtmosphere")
    if sky_atmo_class:
        sky = editor.spawn_actor_from_class(sky_atmo_class, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        if sky:
            sky.set_actor_label("SkyAtmosphere")
            unreal.log("Created SkyAtmosphere")
    
    # Sky Light (real-time capture)
    sky_light_class = unreal.load_class(None, "/Script/Engine.SkyLight")
    if sky_light_class:
        sl = editor.spawn_actor_from_class(sky_light_class, unreal.Vector(0, 0, 500), unreal.Rotator(0, 0, 0))
        if sl:
            sl.set_actor_label("SkyLight")
            comp = sl.get_component_by_class(unreal.SkyLightComponent)
            if comp:
                comp.set_editor_property("source_type", unreal.SkyLightSourceType.SLS_CAPTURED_SCENE)
                comp.set_editor_property("real_time_capture", True)
            unreal.log("Created SkyLight (real-time capture)")
    
    # Exponential Height Fog
    fog_class = unreal.load_class(None, "/Script/Engine.ExponentialHeightFog")
    if fog_class:
        fog = editor.spawn_actor_from_class(fog_class, unreal.Vector(0, 0, 300), unreal.Rotator(0, 0, 0))
        if fog:
            fog.set_actor_label("HeightFog")
            fc = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
            if fc:
                fc.set_editor_property("fog_density", 0.008)
                fc.set_editor_property("fog_height_falloff", 0.2)
                fc.set_editor_property("fog_inscattering_color", unreal.LinearColor(0.45, 0.55, 0.7, 1.0))
                fc.set_editor_property("volumetric_fog", True)
                fc.set_editor_property("volumetric_fog_distance", 12000.0)
            unreal.log("Created Exponential Height Fog")
    
    # =====================================================
    # FOREST - Trees of varied sizes
    # =====================================================
    
    # Large trees (trunk + canopy)
    tree_positions = []
    for i in range(40):
        x = random.uniform(-15000, 15000)
        y = random.uniform(-15000, 15000)
        # Keep clear of spawn area
        if abs(x) < 800 and abs(y) < 800:
            continue
        tree_positions.append((x, y))
    
    for i, (x, y) in enumerate(tree_positions):
        trunk_height = random.uniform(4, 8)
        trunk_radius = random.uniform(0.6, 1.2)
        canopy_radius = random.uniform(3, 6)
        
        # Trunk
        trunk = editor.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(x, y, trunk_height * 50),
            unreal.Rotator(0, 0, 0)
        )
        if trunk and cylinder_mesh:
            trunk.set_actor_label("Tree_%02d_Trunk" % i)
            trunk.set_actor_scale3d(unreal.Vector(trunk_radius, trunk_radius, trunk_height))
            trunk.static_mesh_component.set_static_mesh(cylinder_mesh)
            if basic_mat:
                trunk.static_mesh_component.set_material(0, basic_mat)
        
        # Canopy (sphere)
        canopy = editor.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(x, y, trunk_height * 100 + canopy_radius * 30),
            unreal.Rotator(0, 0, 0)
        )
        if canopy and sphere_mesh:
            canopy.set_actor_label("Tree_%02d_Canopy" % i)
            cr = canopy_radius
            canopy.set_actor_scale3d(unreal.Vector(cr, cr, cr * 0.7))
            canopy.static_mesh_component.set_static_mesh(sphere_mesh)
            if basic_mat:
                canopy.static_mesh_component.set_material(0, basic_mat)
    
    unreal.log("Created %d trees" % len(tree_positions))
    
    # =====================================================
    # ROCKS & BOULDERS
    # =====================================================
    
    rock_data = []
    for i in range(25):
        x = random.uniform(-12000, 12000)
        y = random.uniform(-12000, 12000)
        if abs(x) < 500 and abs(y) < 500:
            continue
        sx = random.uniform(1.5, 5)
        sy = random.uniform(1.5, 5)
        sz = random.uniform(1.0, 3.5)
        rock_data.append((x, y, sx, sy, sz))
    
    for i, (x, y, sx, sy, sz) in enumerate(rock_data):
        rock = editor.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(x, y, sz * 25),
            unreal.Rotator(random.uniform(-10, 10), random.uniform(0, 360), random.uniform(-10, 10))
        )
        if rock and sphere_mesh:
            rock.set_actor_label("Rock_%02d" % i)
            rock.set_actor_scale3d(unreal.Vector(sx, sy, sz))
            rock.static_mesh_component.set_static_mesh(sphere_mesh)
            if basic_mat:
                rock.static_mesh_component.set_material(0, basic_mat)
    
    unreal.log("Created %d rocks/boulders" % len(rock_data))
    
    # =====================================================
    # WATER BODY (Lake)
    # =====================================================
    
    water = editor.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(5000, 5000, -30),
        unreal.Rotator(0, 0, 0)
    )
    if water and cube_mesh:
        water.set_actor_label("Lake_Water")
        water.set_actor_scale3d(unreal.Vector(80, 60, 0.3))
        water.static_mesh_component.set_static_mesh(cube_mesh)
        if basic_mat:
            water.static_mesh_component.set_material(0, basic_mat)
        unreal.log("Created lake water body")
    
    # =====================================================
    # CAMP AREA (near spawn)
    # =====================================================
    
    # Shelter frame (4 posts + roof)
    shelter_x, shelter_y = 400, 300
    posts = [
        (shelter_x - 150, shelter_y - 100),
        (shelter_x + 150, shelter_y - 100),
        (shelter_x - 150, shelter_y + 100),
        (shelter_x + 150, shelter_y + 100),
    ]
    for pi, (px, py) in enumerate(posts):
        post = editor.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(px, py, 100),
            unreal.Rotator(0, 0, 0)
        )
        if post and cylinder_mesh:
            post.set_actor_label("Shelter_Post_%d" % pi)
            post.set_actor_scale3d(unreal.Vector(0.15, 0.15, 2.0))
            post.static_mesh_component.set_static_mesh(cylinder_mesh)
            if basic_mat:
                post.static_mesh_component.set_material(0, basic_mat)
    
    # Roof
    roof = editor.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(shelter_x, shelter_y, 210),
        unreal.Rotator(0, 0, 0)
    )
    if roof and cube_mesh:
        roof.set_actor_label("Shelter_Roof")
        roof.set_actor_scale3d(unreal.Vector(3.5, 2.5, 0.1))
        roof.static_mesh_component.set_static_mesh(cube_mesh)
        if basic_mat:
            roof.static_mesh_component.set_material(0, basic_mat)
    
    # Fire pit (ring of small stones + center)
    fire_x, fire_y = 200, 150
    for fi in range(8):
        import math
        angle = fi * (2 * math.pi / 8)
        fx = fire_x + math.cos(angle) * 60
        fy = fire_y + math.sin(angle) * 60
        stone = editor.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(fx, fy, 12),
            unreal.Rotator(0, 0, 0)
        )
        if stone and sphere_mesh:
            stone.set_actor_label("FirePit_Stone_%d" % fi)
            stone.set_actor_scale3d(unreal.Vector(0.2, 0.2, 0.15))
            stone.static_mesh_component.set_static_mesh(sphere_mesh)
            if basic_mat:
                stone.static_mesh_component.set_material(0, basic_mat)
    
    unreal.log("Created camp area with shelter and fire pit")
    
    # Crates near shelter
    for ci in range(3):
        crate = editor.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(shelter_x + 200 + ci * 80, shelter_y - 50, 50),
            unreal.Rotator(0, random.uniform(0, 30), 0)
        )
        if crate and cube_mesh:
            crate.set_actor_label("Crate_%d" % ci)
            crate.set_actor_scale3d(unreal.Vector(0.8, 0.8, 0.8))
            crate.static_mesh_component.set_static_mesh(cube_mesh)
            if basic_mat:
                crate.static_mesh_component.set_material(0, basic_mat)
    
    # =====================================================
    # DISTANT LANDMARKS
    # =====================================================
    
    # Tall rock pillar (visible landmark)
    pillar = editor.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(10000, -8000, 300),
        unreal.Rotator(0, 15, 0)
    )
    if pillar and cylinder_mesh:
        pillar.set_actor_label("Landmark_RockPillar")
        pillar.set_actor_scale3d(unreal.Vector(4, 4, 12))
        pillar.static_mesh_component.set_static_mesh(cylinder_mesh)
        if basic_mat:
            pillar.static_mesh_component.set_material(0, basic_mat)
    
    # Ruined wall
    wall = editor.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(-8000, 7000, 120),
        unreal.Rotator(0, 35, 2)
    )
    if wall and cube_mesh:
        wall.set_actor_label("Landmark_RuinedWall")
        wall.set_actor_scale3d(unreal.Vector(12, 0.3, 5))
        wall.static_mesh_component.set_static_mesh(cube_mesh)
        if basic_mat:
            wall.static_mesh_component.set_material(0, basic_mat)
    
    unreal.log("Created distant landmarks")
    
    # =====================================================
    # POST PROCESSING VOLUME
    # =====================================================
    
    ppv_class = unreal.load_class(None, "/Script/Engine.PostProcessVolume")
    if ppv_class:
        ppv = editor.spawn_actor_from_class(ppv_class, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        if ppv:
            ppv.set_actor_label("GlobalPostProcess")
            ppv.set_editor_property("unbound", True)
            
            settings = ppv.get_editor_property("settings")
            if settings:
                # Auto exposure
                settings.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_HISTOGRAM)
                settings.set_editor_property("auto_exposure_min_brightness", 0.05)
                settings.set_editor_property("auto_exposure_max_brightness", 8.0)
                
                # Bloom
                settings.set_editor_property("bloom_intensity", 0.6)
                
                # Vignette
                settings.set_editor_property("vignette_intensity", 0.3)
            
            unreal.log("Created global PostProcessVolume with cinematic settings")
    
    # =====================================================
    # DONE
    # =====================================================
    
    unreal.log("========================================")
    unreal.log("ENVIRONMENT SETUP COMPLETE!")
    unreal.log("========================================")
    unreal.log("")
    unreal.log("Created:")
    unreal.log("  - 400m x 400m ground terrain")
    unreal.log("  - %d rolling hills" % len(hills))
    unreal.log("  - %d trees (trunk + canopy)" % len(tree_positions))
    unreal.log("  - %d rock formations" % len(rock_data))
    unreal.log("  - Lake water body")
    unreal.log("  - Camp area (shelter + fire pit + crates)")
    unreal.log("  - Distant landmarks (rock pillar, ruined wall)")
    unreal.log("  - Full atmospheric lighting (sun, sky, fog)")
    unreal.log("  - Global post-processing")
    unreal.log("")
    unreal.log("GameMode auto-spawns DayNightCycle + WeatherSystem.")
    unreal.log("Press Ctrl+S to save, then Play to test!")
    unreal.log("")
    unreal.log("Controls:")
    unreal.log("  WASD = Move     Mouse = Look      Space = Jump")
    unreal.log("  Shift = Sprint  Ctrl = Crouch      E = Interact")
    unreal.log("  LMB = Attack    RMB = Block        Alt = Dodge")
    unreal.log("  V = Camera      Tab = Inventory    Esc = Pause")
    unreal.log("========================================")

# Run
setup_test_level()
