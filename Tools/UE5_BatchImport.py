#!/usr/bin/env python3
"""
BiomeSurvivor - UE5 Batch Asset Import Script
=============================================
Execute this script inside Unreal Editor:
  Tools -> Execute Python Script -> Select this file

This will:
1. Import all PBR textures with correct settings
2. Create Material Instances for each texture set
3. Import all audio files with correct settings
4. Create Sound Cues with randomization/attenuation
5. Set up proper folder structure in Content/

Prerequisites:
- Enable "Python Editor Script Plugin" in UE5
- Enable "Editor Scripting Utilities" plugin

CC0 License - All imported assets are Creative Commons Zero
"""

import unreal
import os

# ─── Helpers ──────────────────────────────────────────────────────────

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_util = unreal.EditorAssetLibrary

def ensure_directory(path):
    """Create content directory if it doesn't exist."""
    if not editor_util.does_directory_exist(path):
        editor_util.make_directory(path)

def import_texture(source_path, dest_path, dest_name, srgb=True, compression="TC_Default", normal_map=False):
    """Import a single texture with proper settings."""
    task = unreal.AssetImportTask()
    task.filename = source_path
    task.destination_path = dest_path
    task.destination_name = dest_name
    task.replace_existing = True
    task.automated = True
    task.save = True

    asset_tools.import_asset_tasks([task])

    # Apply settings after import
    asset_path = f"{dest_path}/{dest_name}"
    texture = editor_util.load_asset(asset_path)
    if texture:
        texture.set_editor_property("srgb", srgb)
        if normal_map:
            texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        elif compression == "TC_Masks":
            texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_WORLD)
        editor_util.save_asset(asset_path)
    return texture

def import_audio(source_path, dest_path, dest_name):
    """Import a single audio file."""
    task = unreal.AssetImportTask()
    task.filename = source_path
    task.destination_path = dest_path
    task.destination_name = dest_name
    task.replace_existing = True
    task.automated = True
    task.save = True

    asset_tools.import_asset_tasks([task])

    asset_path = f"{dest_path}/{dest_name}"
    return editor_util.load_asset(asset_path)

def create_material_instance(parent_path, instance_name, dest_path, textures_map):
    """Create a Material Instance Dynamic with texture parameters."""
    ensure_directory(dest_path)

    parent_material = editor_util.load_asset(parent_path)
    if not parent_material:
        unreal.log_warning(f"Parent material not found: {parent_path}")
        return None

    factory = unreal.MaterialInstanceConstantFactoryNew()
    mi = asset_tools.create_asset(instance_name, dest_path, unreal.MaterialInstanceConstant, factory)

    if mi:
        mi.set_editor_property("parent", parent_material)

        for param_name, texture_asset in textures_map.items():
            if texture_asset:
                unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                    mi, param_name, texture_asset
                )

        editor_util.save_asset(f"{dest_path}/{instance_name}")

    return mi


# ─── Main Import Routine ─────────────────────────────────────────────

def main():
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    unreal.log("=" * 60)
    unreal.log("BiomeSurvivor Asset Import - Starting...")
    unreal.log("=" * 60)

    # Create content folder structure
    content_dirs = [
        "/Game/BiomeSurvivor/Textures/Grass",
        "/Game/BiomeSurvivor/Textures/Snow",
        "/Game/BiomeSurvivor/Textures/Ground",
        "/Game/BiomeSurvivor/Textures/Rock",
        "/Game/BiomeSurvivor/Textures/Sand",
        "/Game/BiomeSurvivor/Textures/Wood",
        "/Game/BiomeSurvivor/Textures/Metal",
        "/Game/BiomeSurvivor/Materials/Landscape",
        "/Game/BiomeSurvivor/Materials/Props",
        "/Game/BiomeSurvivor/Audio/Footsteps",
        "/Game/BiomeSurvivor/Audio/Ambient",
        "/Game/BiomeSurvivor/Audio/Combat",
        "/Game/BiomeSurvivor/Audio/UI",
        "/Game/BiomeSurvivor/Audio/Weather",
        "/Game/BiomeSurvivor/Audio/Wildlife",
        "/Game/BiomeSurvivor/Audio/Crafting",
        "/Game/BiomeSurvivor/Audio/Music",
        "/Game/BiomeSurvivor/SoundCues",
    ]
    for d in content_dirs:
        ensure_directory(d)

    # ─── Import Textures ──────────────────────────────────────────
    unreal.log("Importing PBR Textures...")

    # --- Grass001_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Grass/Grass001_1K-JPG")
    unreal.log("Importing Grass001_1K-JPG...")
    tex_Grass001_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass001_1K-JPG/Grass001.png"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass001_1K-JPG", "Grass001",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Grass001_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass001_1K-JPG/Grass001_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass001_1K-JPG", "Grass001_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Grass001_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass001_1K-JPG/Grass001_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass001_1K-JPG", "Grass001_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Grass001_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass001_1K-JPG/Grass001_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass001_1K-JPG", "Grass001_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Grass001_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass001_1K-JPG/Grass001_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass001_1K-JPG", "Grass001_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Grass001_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass001_1K-JPG/Grass001_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass001_1K-JPG", "Grass001_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Grass001_1K-JPG
    mi_textures = {
        "BaseColor": tex_Grass001_1K_JPG_BaseColor,
        "Normal": tex_Grass001_1K_JPG_Normal,
        "Roughness": tex_Grass001_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Grass001_1K_JPG_AO,
        "Displacement": tex_Grass001_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Grass001_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Grass004_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Grass/Grass004_1K-JPG")
    unreal.log("Importing Grass004_1K-JPG...")
    tex_Grass004_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass004_1K-JPG/Grass004.png"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass004_1K-JPG", "Grass004",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Grass004_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass004_1K-JPG/Grass004_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass004_1K-JPG", "Grass004_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Grass004_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass004_1K-JPG/Grass004_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass004_1K-JPG", "Grass004_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Grass004_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass004_1K-JPG/Grass004_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass004_1K-JPG", "Grass004_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Grass004_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass004_1K-JPG/Grass004_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass004_1K-JPG", "Grass004_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Grass004_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Grass004_1K-JPG/Grass004_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Grass004_1K-JPG", "Grass004_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Grass004_1K-JPG
    mi_textures = {
        "BaseColor": tex_Grass004_1K_JPG_BaseColor,
        "Normal": tex_Grass004_1K_JPG_Normal,
        "Roughness": tex_Grass004_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Grass004_1K_JPG_AO,
        "Displacement": tex_Grass004_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Grass004_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Moss001_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Grass/Moss001_1K-JPG")
    unreal.log("Importing Moss001_1K-JPG...")
    tex_Moss001_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Moss001_1K-JPG/Moss001.png"),
        "/Game/BiomeSurvivor/Textures/Grass/Moss001_1K-JPG", "Moss001",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Moss001_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Moss001_1K-JPG/Moss001_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Moss001_1K-JPG", "Moss001_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Moss001_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Moss001_1K-JPG/Moss001_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Moss001_1K-JPG", "Moss001_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Moss001_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Moss001_1K-JPG/Moss001_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Moss001_1K-JPG", "Moss001_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Moss001_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Moss001_1K-JPG/Moss001_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Moss001_1K-JPG", "Moss001_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Moss001_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Grass/Moss001_1K-JPG/Moss001_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Grass/Moss001_1K-JPG", "Moss001_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Moss001_1K-JPG
    mi_textures = {
        "BaseColor": tex_Moss001_1K_JPG_BaseColor,
        "Normal": tex_Moss001_1K_JPG_Normal,
        "Roughness": tex_Moss001_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Moss001_1K_JPG_AO,
        "Displacement": tex_Moss001_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Moss001_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Ground037_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Ground/Ground037_1K-JPG")
    unreal.log("Importing Ground037_1K-JPG...")
    tex_Ground037_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground037_1K-JPG/Ground037.png"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground037_1K-JPG", "Ground037",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Ground037_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground037_1K-JPG/Ground037_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground037_1K-JPG", "Ground037_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Ground037_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground037_1K-JPG/Ground037_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground037_1K-JPG", "Ground037_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Ground037_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground037_1K-JPG/Ground037_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground037_1K-JPG", "Ground037_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Ground037_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground037_1K-JPG/Ground037_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground037_1K-JPG", "Ground037_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Ground037_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground037_1K-JPG/Ground037_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground037_1K-JPG", "Ground037_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Ground037_1K-JPG
    mi_textures = {
        "BaseColor": tex_Ground037_1K_JPG_BaseColor,
        "Normal": tex_Ground037_1K_JPG_Normal,
        "Roughness": tex_Ground037_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Ground037_1K_JPG_AO,
        "Displacement": tex_Ground037_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Ground037_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Ground103_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Ground/Ground103_1K-JPG")
    unreal.log("Importing Ground103_1K-JPG...")
    tex_Ground103_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground103_1K-JPG/Ground103.png"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground103_1K-JPG", "Ground103",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Ground103_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground103_1K-JPG/Ground103_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground103_1K-JPG", "Ground103_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Ground103_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground103_1K-JPG/Ground103_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground103_1K-JPG", "Ground103_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Ground103_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground103_1K-JPG/Ground103_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground103_1K-JPG", "Ground103_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Ground103_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground103_1K-JPG/Ground103_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground103_1K-JPG", "Ground103_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Ground103_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Ground/Ground103_1K-JPG/Ground103_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Ground/Ground103_1K-JPG", "Ground103_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Ground103_1K-JPG
    mi_textures = {
        "BaseColor": tex_Ground103_1K_JPG_BaseColor,
        "Normal": tex_Ground103_1K_JPG_Normal,
        "Roughness": tex_Ground103_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Ground103_1K_JPG_AO,
        "Displacement": tex_Ground103_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Ground103_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Metal034_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Metal/Metal034_1K-JPG")
    unreal.log("Importing Metal034_1K-JPG...")
    tex_Metal034_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal034_1K-JPG/Metal034.png"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal034_1K-JPG", "Metal034",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Metal034_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal034_1K-JPG/Metal034_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal034_1K-JPG", "Metal034_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Metal034_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal034_1K-JPG/Metal034_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal034_1K-JPG", "Metal034_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Metal034_1K_JPG_Metallic = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal034_1K-JPG/Metal034_1K-JPG_Metalness.jpg"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal034_1K-JPG", "Metal034_1K-JPG_Metalness",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Metal034_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal034_1K-JPG/Metal034_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal034_1K-JPG", "Metal034_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Metal034_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal034_1K-JPG/Metal034_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal034_1K-JPG", "Metal034_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Metal034_1K-JPG
    mi_textures = {
        "BaseColor": tex_Metal034_1K_JPG_BaseColor,
        "Normal": tex_Metal034_1K_JPG_Normal,
        "Roughness": tex_Metal034_1K_JPG_Roughness,
        "Metallic": tex_Metal034_1K_JPG_Metallic,
        "Displacement": tex_Metal034_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Metal034_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Metal038_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Metal/Metal038_1K-JPG")
    unreal.log("Importing Metal038_1K-JPG...")
    tex_Metal038_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal038_1K-JPG/Metal038.png"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal038_1K-JPG", "Metal038",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Metal038_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal038_1K-JPG/Metal038_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal038_1K-JPG", "Metal038_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Metal038_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal038_1K-JPG/Metal038_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal038_1K-JPG", "Metal038_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Metal038_1K_JPG_Metallic = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal038_1K-JPG/Metal038_1K-JPG_Metalness.jpg"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal038_1K-JPG", "Metal038_1K-JPG_Metalness",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Metal038_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal038_1K-JPG/Metal038_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal038_1K-JPG", "Metal038_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Metal038_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Metal/Metal038_1K-JPG/Metal038_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Metal/Metal038_1K-JPG", "Metal038_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Metal038_1K-JPG
    mi_textures = {
        "BaseColor": tex_Metal038_1K_JPG_BaseColor,
        "Normal": tex_Metal038_1K_JPG_Normal,
        "Roughness": tex_Metal038_1K_JPG_Roughness,
        "Metallic": tex_Metal038_1K_JPG_Metallic,
        "Displacement": tex_Metal038_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Metal038_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Rock030_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Rock/Rock030_1K-JPG")
    unreal.log("Importing Rock030_1K-JPG...")
    tex_Rock030_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock030_1K-JPG/Rock030.png"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock030_1K-JPG", "Rock030",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Rock030_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock030_1K-JPG/Rock030_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock030_1K-JPG", "Rock030_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Rock030_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock030_1K-JPG/Rock030_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock030_1K-JPG", "Rock030_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Rock030_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock030_1K-JPG/Rock030_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock030_1K-JPG", "Rock030_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Rock030_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock030_1K-JPG/Rock030_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock030_1K-JPG", "Rock030_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Rock030_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock030_1K-JPG/Rock030_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock030_1K-JPG", "Rock030_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Rock030_1K-JPG
    mi_textures = {
        "BaseColor": tex_Rock030_1K_JPG_BaseColor,
        "Normal": tex_Rock030_1K_JPG_Normal,
        "Roughness": tex_Rock030_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Rock030_1K_JPG_AO,
        "Displacement": tex_Rock030_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Rock030_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Rock034_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Rock/Rock034_1K-JPG")
    unreal.log("Importing Rock034_1K-JPG...")
    tex_Rock034_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock034_1K-JPG/Rock034.png"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock034_1K-JPG", "Rock034",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Rock034_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock034_1K-JPG/Rock034_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock034_1K-JPG", "Rock034_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Rock034_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock034_1K-JPG/Rock034_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock034_1K-JPG", "Rock034_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Rock034_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock034_1K-JPG/Rock034_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock034_1K-JPG", "Rock034_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Rock034_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock034_1K-JPG/Rock034_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock034_1K-JPG", "Rock034_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Rock034_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Rock/Rock034_1K-JPG/Rock034_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Rock/Rock034_1K-JPG", "Rock034_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Rock034_1K-JPG
    mi_textures = {
        "BaseColor": tex_Rock034_1K_JPG_BaseColor,
        "Normal": tex_Rock034_1K_JPG_Normal,
        "Roughness": tex_Rock034_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Rock034_1K_JPG_AO,
        "Displacement": tex_Rock034_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Rock034_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Gravel017_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Sand/Gravel017_1K-JPG")
    unreal.log("Importing Gravel017_1K-JPG...")
    tex_Gravel017_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel017_1K-JPG/Gravel017_1K-JPG_IdMask.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel017_1K-JPG", "Gravel017_1K-JPG_IdMask",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Gravel017_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel017_1K-JPG/Gravel017_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel017_1K-JPG", "Gravel017_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Gravel017_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel017_1K-JPG/Gravel017_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel017_1K-JPG", "Gravel017_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Gravel017_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel017_1K-JPG/Gravel017_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel017_1K-JPG", "Gravel017_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Gravel017_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel017_1K-JPG/Gravel017_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel017_1K-JPG", "Gravel017_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Gravel017_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel017_1K-JPG/Gravel017_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel017_1K-JPG", "Gravel017_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Gravel017_1K-JPG
    mi_textures = {
        "BaseColor": tex_Gravel017_1K_JPG_BaseColor,
        "Normal": tex_Gravel017_1K_JPG_Normal,
        "Roughness": tex_Gravel017_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Gravel017_1K_JPG_AO,
        "Displacement": tex_Gravel017_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Gravel017_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Gravel022_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Sand/Gravel022_1K-JPG")
    unreal.log("Importing Gravel022_1K-JPG...")
    tex_Gravel022_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel022_1K-JPG/Gravel022.png"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel022_1K-JPG", "Gravel022",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Gravel022_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel022_1K-JPG/Gravel022_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel022_1K-JPG", "Gravel022_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Gravel022_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel022_1K-JPG/Gravel022_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel022_1K-JPG", "Gravel022_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Gravel022_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel022_1K-JPG/Gravel022_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel022_1K-JPG", "Gravel022_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Gravel022_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel022_1K-JPG/Gravel022_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel022_1K-JPG", "Gravel022_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Gravel022_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel022_1K-JPG/Gravel022_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel022_1K-JPG", "Gravel022_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Gravel022_1K-JPG
    mi_textures = {
        "BaseColor": tex_Gravel022_1K_JPG_BaseColor,
        "Normal": tex_Gravel022_1K_JPG_Normal,
        "Roughness": tex_Gravel022_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Gravel022_1K_JPG_AO,
        "Displacement": tex_Gravel022_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Gravel022_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Gravel026_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Sand/Gravel026_1K-JPG")
    unreal.log("Importing Gravel026_1K-JPG...")
    tex_Gravel026_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel026_1K-JPG/Gravel026.png"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel026_1K-JPG", "Gravel026",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Gravel026_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel026_1K-JPG/Gravel026_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel026_1K-JPG", "Gravel026_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Gravel026_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel026_1K-JPG/Gravel026_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel026_1K-JPG", "Gravel026_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Gravel026_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel026_1K-JPG/Gravel026_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel026_1K-JPG", "Gravel026_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Gravel026_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel026_1K-JPG/Gravel026_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel026_1K-JPG", "Gravel026_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Gravel026_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Gravel026_1K-JPG/Gravel026_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Gravel026_1K-JPG", "Gravel026_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Gravel026_1K-JPG
    mi_textures = {
        "BaseColor": tex_Gravel026_1K_JPG_BaseColor,
        "Normal": tex_Gravel026_1K_JPG_Normal,
        "Roughness": tex_Gravel026_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Gravel026_1K_JPG_AO,
        "Displacement": tex_Gravel026_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Gravel026_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Ground046_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Sand/Ground046_1K-JPG")
    unreal.log("Importing Ground046_1K-JPG...")
    tex_Ground046_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground046_1K-JPG/Ground046.png"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground046_1K-JPG", "Ground046",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Ground046_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground046_1K-JPG/Ground046_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground046_1K-JPG", "Ground046_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Ground046_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground046_1K-JPG/Ground046_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground046_1K-JPG", "Ground046_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Ground046_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground046_1K-JPG/Ground046_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground046_1K-JPG", "Ground046_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Ground046_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground046_1K-JPG/Ground046_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground046_1K-JPG", "Ground046_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Ground046_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground046_1K-JPG/Ground046_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground046_1K-JPG", "Ground046_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Ground046_1K-JPG
    mi_textures = {
        "BaseColor": tex_Ground046_1K_JPG_BaseColor,
        "Normal": tex_Ground046_1K_JPG_Normal,
        "Roughness": tex_Ground046_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Ground046_1K_JPG_AO,
        "Displacement": tex_Ground046_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Ground046_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Ground054_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Sand/Ground054_1K-JPG")
    unreal.log("Importing Ground054_1K-JPG...")
    tex_Ground054_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground054_1K-JPG/Ground054.png"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground054_1K-JPG", "Ground054",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Ground054_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground054_1K-JPG/Ground054_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground054_1K-JPG", "Ground054_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Ground054_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground054_1K-JPG/Ground054_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground054_1K-JPG", "Ground054_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Ground054_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground054_1K-JPG/Ground054_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground054_1K-JPG", "Ground054_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Ground054_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground054_1K-JPG/Ground054_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground054_1K-JPG", "Ground054_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Ground054_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Sand/Ground054_1K-JPG/Ground054_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Sand/Ground054_1K-JPG", "Ground054_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Ground054_1K-JPG
    mi_textures = {
        "BaseColor": tex_Ground054_1K_JPG_BaseColor,
        "Normal": tex_Ground054_1K_JPG_Normal,
        "Roughness": tex_Ground054_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Ground054_1K_JPG_AO,
        "Displacement": tex_Ground054_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Ground054_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Snow013_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Snow/Snow013_1K-JPG")
    unreal.log("Importing Snow013_1K-JPG...")
    tex_Snow013_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow013_1K-JPG/Snow013.png"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow013_1K-JPG", "Snow013",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Snow013_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow013_1K-JPG/Snow013_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow013_1K-JPG", "Snow013_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Snow013_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow013_1K-JPG/Snow013_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow013_1K-JPG", "Snow013_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Snow013_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow013_1K-JPG/Snow013_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow013_1K-JPG", "Snow013_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Snow013_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow013_1K-JPG/Snow013_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow013_1K-JPG", "Snow013_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Snow013_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow013_1K-JPG/Snow013_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow013_1K-JPG", "Snow013_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Snow013_1K-JPG
    mi_textures = {
        "BaseColor": tex_Snow013_1K_JPG_BaseColor,
        "Normal": tex_Snow013_1K_JPG_Normal,
        "Roughness": tex_Snow013_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Snow013_1K_JPG_AO,
        "Displacement": tex_Snow013_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Snow013_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Snow014_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Snow/Snow014_1K-JPG")
    unreal.log("Importing Snow014_1K-JPG...")
    tex_Snow014_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow014_1K-JPG/Snow014.png"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow014_1K-JPG", "Snow014",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Snow014_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow014_1K-JPG/Snow014_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow014_1K-JPG", "Snow014_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Snow014_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow014_1K-JPG/Snow014_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow014_1K-JPG", "Snow014_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Snow014_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow014_1K-JPG/Snow014_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow014_1K-JPG", "Snow014_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Snow014_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow014_1K-JPG/Snow014_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow014_1K-JPG", "Snow014_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Snow014_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Snow/Snow014_1K-JPG/Snow014_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Snow/Snow014_1K-JPG", "Snow014_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Snow014_1K-JPG
    mi_textures = {
        "BaseColor": tex_Snow014_1K_JPG_BaseColor,
        "Normal": tex_Snow014_1K_JPG_Normal,
        "Roughness": tex_Snow014_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Snow014_1K_JPG_AO,
        "Displacement": tex_Snow014_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Snow014_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- Bark006_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Wood/Bark006_1K-JPG")
    unreal.log("Importing Bark006_1K-JPG...")
    tex_Bark006_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/Bark006_1K-JPG/Bark006.png"),
        "/Game/BiomeSurvivor/Textures/Wood/Bark006_1K-JPG", "Bark006",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Bark006_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/Bark006_1K-JPG/Bark006_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Wood/Bark006_1K-JPG", "Bark006_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_Bark006_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/Bark006_1K-JPG/Bark006_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Wood/Bark006_1K-JPG", "Bark006_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_Bark006_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/Bark006_1K-JPG/Bark006_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Wood/Bark006_1K-JPG", "Bark006_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_Bark006_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/Bark006_1K-JPG/Bark006_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Wood/Bark006_1K-JPG", "Bark006_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_Bark006_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/Bark006_1K-JPG/Bark006_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Wood/Bark006_1K-JPG", "Bark006_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_Bark006_1K-JPG
    mi_textures = {
        "BaseColor": tex_Bark006_1K_JPG_BaseColor,
        "Normal": tex_Bark006_1K_JPG_Normal,
        "Roughness": tex_Bark006_1K_JPG_Roughness,
        "AmbientOcclusion": tex_Bark006_1K_JPG_AO,
        "Displacement": tex_Bark006_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_Bark006_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )

    # --- WoodFloor040_1K-JPG ---
    ensure_directory("/Game/BiomeSurvivor/Textures/Wood/WoodFloor040_1K-JPG")
    unreal.log("Importing WoodFloor040_1K-JPG...")
    tex_WoodFloor040_1K_JPG_Unknown = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/WoodFloor040_1K-JPG/WoodFloor040.png"),
        "/Game/BiomeSurvivor/Textures/Wood/WoodFloor040_1K-JPG", "WoodFloor040",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_WoodFloor040_1K_JPG_AO = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/WoodFloor040_1K-JPG/WoodFloor040_1K-JPG_AmbientOcclusion.jpg"),
        "/Game/BiomeSurvivor/Textures/Wood/WoodFloor040_1K-JPG", "WoodFloor040_1K-JPG_AmbientOcclusion",
        srgb=False, compression="TC_Masks", normal_map=False
    )
    tex_WoodFloor040_1K_JPG_BaseColor = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/WoodFloor040_1K-JPG/WoodFloor040_1K-JPG_Color.jpg"),
        "/Game/BiomeSurvivor/Textures/Wood/WoodFloor040_1K-JPG", "WoodFloor040_1K-JPG_Color",
        srgb=True, compression="TC_Default", normal_map=False
    )
    tex_WoodFloor040_1K_JPG_Displacement = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/WoodFloor040_1K-JPG/WoodFloor040_1K-JPG_Displacement.jpg"),
        "/Game/BiomeSurvivor/Textures/Wood/WoodFloor040_1K-JPG", "WoodFloor040_1K-JPG_Displacement",
        srgb=False, compression="TC_Displacementmap", normal_map=False
    )
    tex_WoodFloor040_1K_JPG_Normal = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/WoodFloor040_1K-JPG/WoodFloor040_1K-JPG_NormalGL.jpg"),
        "/Game/BiomeSurvivor/Textures/Wood/WoodFloor040_1K-JPG", "WoodFloor040_1K-JPG_NormalGL",
        srgb=False, compression="TC_Normalmap", normal_map=True
    )
    tex_WoodFloor040_1K_JPG_Roughness = import_texture(
        os.path.join(project_root, "Assets/Textures/Wood/WoodFloor040_1K-JPG/WoodFloor040_1K-JPG_Roughness.jpg"),
        "/Game/BiomeSurvivor/Textures/Wood/WoodFloor040_1K-JPG", "WoodFloor040_1K-JPG_Roughness",
        srgb=False, compression="TC_Masks", normal_map=False
    )

    # Create Material Instance: MI_WoodFloor040_1K-JPG
    mi_textures = {
        "BaseColor": tex_WoodFloor040_1K_JPG_BaseColor,
        "Normal": tex_WoodFloor040_1K_JPG_Normal,
        "Roughness": tex_WoodFloor040_1K_JPG_Roughness,
        "AmbientOcclusion": tex_WoodFloor040_1K_JPG_AO,
        "Displacement": tex_WoodFloor040_1K_JPG_Displacement,
    }
    create_material_instance(
        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",
        "MI_WoodFloor040_1K-JPG", "/Game/BiomeSurvivor/Materials/Landscape", mi_textures
    )


    # ─── Import Audio ─────────────────────────────────────────────
    unreal.log("Importing Audio Files...")

    # --- Ambient (31 files) ---
    ensure_directory("/Game/BiomeSurvivor/Audio/Ambient")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/fire_loop.wav"), "/Game/BiomeSurvivor/Audio/Ambient", "fire_loop")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/alarm_01.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "alarm_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/alarm_02.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "alarm_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/alarm_03.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "alarm_03")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/ambient_01.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "ambient_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/ambient_02.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "ambient_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/ambient_03.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "ambient_03")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_01.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_02.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_03.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_03")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_04.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_04")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_05.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_05")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_06.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_06")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_07.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_07")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_08.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_08")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_09.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_09")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_10.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_10")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/machine_11.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "machine_11")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/noise_01.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "noise_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/noise_02.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "noise_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/noise_03.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "noise_03")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/pump_01.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "pump_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/pump_02.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "pump_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/rain.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "rain")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/rolling.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "rolling")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/saw.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "saw")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/water_boiling.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "water_boiling")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/water_flowing.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "water_flowing")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/weird_01.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "weird_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/weird_02.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "weird_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Ambient/sfx_loops/weird_03.ogg"), "/Game/BiomeSurvivor/Audio/Ambient/sfx_loops", "weird_03")

    # --- Combat (123 files) ---
    ensure_directory("/Game/BiomeSurvivor/Audio/Combat")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-1.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_1")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-10.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_10")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-11.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_11")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-12.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_12")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-13.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_13")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-2.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_2")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-3.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_3")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-4.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_4")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-5.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_5")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-6.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_6")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-7.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_7")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-8.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_8")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/__MACOSX/swishes/._swish-9.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\__MACOSX\swishes", "._swish_9")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-1.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_1")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-10.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_10")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-11.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_11")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-12.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_12")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-13.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_13")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-2.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_2")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-3.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_3")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-4.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_4")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-5.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_5")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-6.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_6")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-7.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_7")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-8.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_8")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/swishes/swishes/swish-9.wav"), "/Game/BiomeSurvivor/Audio/Combat/swishes\swishes", "swish_9")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/preview.ogg"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx", "preview")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/apple-cut-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "apple_cut_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/apple-cut-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "apple_cut_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/apple-cut-03.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "apple_cut_03")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/arrow-feathers-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "arrow_feathers_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/arrow-grab-from-quiver-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "arrow_grab_from_quiver_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/arrow-grab-from-quiver-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "arrow_grab_from_quiver_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/arrow-return-to-quiver-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "arrow_return_to_quiver_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/arrow-return-to-quiver-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "arrow_return_to_quiver_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/book-page-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "book_page_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/book-page-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "book_page_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/boots-leather-jump-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "boots_leather_jump_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/boots-leather-step-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "boots_leather_step_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/bottle-clay-uncork-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "bottle_clay_uncork_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/bottle-glass-cork-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "bottle_glass_cork_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/bottle-glass-cork-twist-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "bottle_glass_cork_twist_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/bottle-glass-uncork-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "bottle_glass_uncork_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/bottle-pills-shake-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "bottle_pills_shake_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/bottle-pills-shake-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "bottle_pills_shake_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/bottle-plastic-lid-screw-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "bottle_plastic_lid_screw_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/bottle-plastic-lid-unscrew-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "bottle_plastic_lid_unscrew_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/chimes-wood-rattle.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "chimes_wood_rattle")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/cloth-pouch-shake-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "cloth_pouch_shake_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/cloth-pouch-shake-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "cloth_pouch_shake_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/coin-spin-fall-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "coin_spin_fall_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/coinflip-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "coinflip_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/coins-shake-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "coins_shake_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/compressed-air-spray-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "compressed_air_spray_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/compressed-air-spray-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "compressed_air_spray_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/cover-paper-tear-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "cover_paper_tear_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/cover-paper-tear-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "cover_paper_tear_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/drawer-close-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "drawer_close_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/drawer-open-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "drawer_open_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/floor-creak-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "floor_creak_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/handcuffs-metal-lock-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "handcuffs_metal_lock_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/handcuffs-metal-lock-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "handcuffs_metal_lock_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/handcuffs-metal-open-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "handcuffs_metal_open_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/handsaw-knife-scrape-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "handsaw_knife_scrape_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/handsaw-knife-scrape-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "handsaw_knife_scrape_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/handsaw-sawing-wood-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "handsaw_sawing_wood_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/keyhole-lockbox-insert-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "keyhole_lockbox_insert_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/keyhole-lockbox-remove-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "keyhole_lockbox_remove_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/keyhole-lockbox-turn-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "keyhole_lockbox_turn_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/knife-sheathe-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "knife_sheathe_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/knife-unsheathe-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "knife_unsheathe_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/lighter-light-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "lighter_light_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/match-light-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "match_light_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/metal-fork-scrape-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "metal_fork_scrape_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/metal-fork-scrape-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "metal_fork_scrape_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/metal-hammer-hit-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "metal_hammer_hit_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/metal-hammer-hit-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "metal_hammer_hit_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/metal-knife-scrape-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "metal_knife_scrape_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/metal-knife-scrape-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "metal_knife_scrape_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/metal-knife-scrape-03.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "metal_knife_scrape_03")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/mud-steps-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "mud_steps_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/mud-steps-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "mud_steps_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/mud-steps-03.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "mud_steps_03")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/mud-steps-04.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "mud_steps_04")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/mud-steps-05.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "mud_steps_05")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/paralyzer-discharge-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "paralyzer_discharge_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/paralyzer-discharge-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "paralyzer_discharge_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/plastic-bag-drop-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "plastic_bag_drop_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/plastic-bag-drop-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "plastic_bag_drop_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/plastic-bag-pickup-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "plastic_bag_pickup_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/plastic-bag-pickup-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "plastic_bag_pickup_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/quiver-leather-squeeze-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "quiver_leather_squeeze_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/quiver-leather-squeeze-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "quiver_leather_squeeze_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/scissors-close-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "scissors_close_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/seax-sheathe-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "seax_sheathe_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/seax-sheathe-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "seax_sheathe_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/seax-unsheathe-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "seax_unsheathe_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/seax-unsheathe-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "seax_unsheathe_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/sheath-squeeze-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "sheath_squeeze_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/soil-steps-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "soil_steps_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/sword-clash-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "sword_clash_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/sword-clash-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "sword_clash_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/sword-clash-03.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "sword_clash_03")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/sword-clash-04.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "sword_clash_04")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/sword-clash-05.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "sword_clash_05")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/tin-whistle-whoosh-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "tin_whistle_whoosh_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/tin-whistle-whoosh-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "tin_whistle_whoosh_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/tube-plastic-whoosh-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "tube_plastic_whoosh_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/tube-plastic-whoosh-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "tube_plastic_whoosh_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/vial-glass-close-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "vial_glass_close_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/vial-glass-open-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "vial_glass_open_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/vial-glass-uncork-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "vial_glass_uncork_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/water-drop-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "water_drop_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/water-drop-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "water_drop_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/water-drop-03.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "water_drop_03")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/water-pour-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "water_pour_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/water-vial-fill-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "water_vial_fill_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/wood-bowl-spoon-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "wood_bowl_spoon_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/wood-bowl-spoon-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "wood_bowl_spoon_02")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/wood-bowl-spoon-03.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "wood_bowl_spoon_03")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/wood-bowl-spoon-04.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "wood_bowl_spoon_04")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/wood-twigs-break-01.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "wood_twigs_break_01")
    import_audio(os.path.join(project_root, "Assets/Audio/Combat/tinysized_fantasy_sfx/sfx-cc0/wood-twigs-break-02.wav"), "/Game/BiomeSurvivor/Audio/Combat/tinysized_fantasy_sfx\sfx-cc0", "wood_twigs_break_02")

    # --- Footsteps (8 files) ---
    ensure_directory("/Game/BiomeSurvivor/Audio/Footsteps")
    import_audio(os.path.join(project_root, "Assets/Audio/Footsteps/DifferentSteps/gravel.ogg"), "/Game/BiomeSurvivor/Audio/Footsteps/DifferentSteps", "gravel")
    import_audio(os.path.join(project_root, "Assets/Audio/Footsteps/DifferentSteps/leaves01.ogg"), "/Game/BiomeSurvivor/Audio/Footsteps/DifferentSteps", "leaves01")
    import_audio(os.path.join(project_root, "Assets/Audio/Footsteps/DifferentSteps/leaves02.ogg"), "/Game/BiomeSurvivor/Audio/Footsteps/DifferentSteps", "leaves02")
    import_audio(os.path.join(project_root, "Assets/Audio/Footsteps/DifferentSteps/mud02.ogg"), "/Game/BiomeSurvivor/Audio/Footsteps/DifferentSteps", "mud02")
    import_audio(os.path.join(project_root, "Assets/Audio/Footsteps/DifferentSteps/stone01.ogg"), "/Game/BiomeSurvivor/Audio/Footsteps/DifferentSteps", "stone01")
    import_audio(os.path.join(project_root, "Assets/Audio/Footsteps/DifferentSteps/wood01.ogg"), "/Game/BiomeSurvivor/Audio/Footsteps/DifferentSteps", "wood01")
    import_audio(os.path.join(project_root, "Assets/Audio/Footsteps/DifferentSteps/wood02.ogg"), "/Game/BiomeSurvivor/Audio/Footsteps/DifferentSteps", "wood02")
    import_audio(os.path.join(project_root, "Assets/Audio/Footsteps/DifferentSteps/wood03.ogg"), "/Game/BiomeSurvivor/Audio/Footsteps/DifferentSteps", "wood03")

    # --- RPG (243 files) ---
    ensure_directory("/Game/BiomeSurvivor/Audio/RPG")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/battle/._magic1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\battle", "._magic1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/battle/._spell.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\battle", "._spell")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/battle/._swing.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\battle", "._swing")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/battle/._swing2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\battle", "._swing2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/battle/._swing3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\battle", "._swing3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/battle/._sword-unsheathe.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\battle", "._sword_unsheathe")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/battle/._sword-unsheathe2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\battle", "._sword_unsheathe2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/battle/._sword-unsheathe3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\battle", "._sword_unsheathe3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/battle/._sword-unsheathe4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\battle", "._sword_unsheathe4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/battle/._sword-unsheathe5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\battle", "._sword_unsheathe5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/interface/._interface1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\interface", "._interface1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/interface/._interface2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\interface", "._interface2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/interface/._interface3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\interface", "._interface3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/interface/._interface4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\interface", "._interface4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/interface/._interface5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\interface", "._interface5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/interface/._interface6.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\interface", "._interface6")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._armor-light.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._armor_light")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._beads.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._beads")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._bottle.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._bottle")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._bubble.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._bubble")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._bubble2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._bubble2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._bubble3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._bubble3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._chainmail1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._chainmail1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._chainmail2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._chainmail2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._cloth-heavy.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._cloth_heavy")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._cloth.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._cloth")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._coin.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._coin")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._coin2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._coin2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._coin3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._coin3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._metal-ringing.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._metal_ringing")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._metal-small1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._metal_small1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._metal-small2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._metal_small2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._metal-small3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._metal_small3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/inventory/._wood-small.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\inventory", "._wood_small")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/misc/._burp.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\misc", "._burp")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/misc/._random1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\misc", "._random1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/misc/._random2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\misc", "._random2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/misc/._random3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\misc", "._random3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/misc/._random4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\misc", "._random4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/misc/._random5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\misc", "._random5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/misc/._random6.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\misc", "._random6")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/beetle/._bite-small.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\beetle", "._bite_small")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/beetle/._bite-small2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\beetle", "._bite_small2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/beetle/._bite-small3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\beetle", "._bite_small3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/giant/._giant1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\giant", "._giant1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/giant/._giant2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\giant", "._giant2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/giant/._giant3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\giant", "._giant3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/giant/._giant4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\giant", "._giant4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/giant/._giant5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\giant", "._giant5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr10.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr10")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr11.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr11")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr12.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr12")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr13.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr13")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr14.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr14")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr15.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr15")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr6.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr6")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr7.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr7")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr8.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr8")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/gutteral beast/._mnstr9.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\gutteral beast", "._mnstr9")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/misc/._wolfman.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\misc", "._wolfman")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/ogre/._ogre1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\ogre", "._ogre1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/ogre/._ogre2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\ogre", "._ogre2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/ogre/._ogre3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\ogre", "._ogre3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/ogre/._ogre4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\ogre", "._ogre4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/ogre/._ogre5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\ogre", "._ogre5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade10.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade10")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade11.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade11")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade12.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade12")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade13.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade13")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade14.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade14")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade15.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade15")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade6.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade6")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade7.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade7")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade8.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade8")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/shade/._shade9.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\shade", "._shade9")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/slime/._slime1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\slime", "._slime1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/slime/._slime10.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\slime", "._slime10")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/slime/._slime2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\slime", "._slime2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/slime/._slime3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\slime", "._slime3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/slime/._slime4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\slime", "._slime4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/slime/._slime5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\slime", "._slime5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/slime/._slime6.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\slime", "._slime6")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/slime/._slime7.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\slime", "._slime7")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/slime/._slime8.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\slime", "._slime8")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/NPC/slime/._slime9.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\NPC\slime", "._slime9")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/__MACOSX/RPG Sound Pack/world/._door.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\__MACOSX\RPG Sound Pack\world", "._door")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/battle/magic1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\battle", "magic1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/battle/spell.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\battle", "spell")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/battle/swing.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\battle", "swing")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/battle/swing2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\battle", "swing2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/battle/swing3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\battle", "swing3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/battle/sword-unsheathe.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\battle", "sword_unsheathe")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/battle/sword-unsheathe2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\battle", "sword_unsheathe2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/battle/sword-unsheathe3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\battle", "sword_unsheathe3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/battle/sword-unsheathe4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\battle", "sword_unsheathe4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/battle/sword-unsheathe5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\battle", "sword_unsheathe5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/interface/interface1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\interface", "interface1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/interface/interface2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\interface", "interface2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/interface/interface3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\interface", "interface3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/interface/interface4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\interface", "interface4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/interface/interface5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\interface", "interface5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/interface/interface6.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\interface", "interface6")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/armor-light.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "armor_light")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/beads.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "beads")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/bottle.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "bottle")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/bubble.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "bubble")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/bubble2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "bubble2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/bubble3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "bubble3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/chainmail1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "chainmail1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/chainmail2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "chainmail2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/cloth-heavy.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "cloth_heavy")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/cloth.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "cloth")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/coin.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "coin")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/coin2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "coin2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/coin3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "coin3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/metal-ringing.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "metal_ringing")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/metal-small1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "metal_small1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/metal-small2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "metal_small2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/metal-small3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "metal_small3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/inventory/wood-small.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\inventory", "wood_small")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/misc/burp.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\misc", "burp")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/misc/random1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\misc", "random1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/misc/random2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\misc", "random2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/misc/random3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\misc", "random3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/misc/random4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\misc", "random4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/misc/random5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\misc", "random5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/misc/random6.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\misc", "random6")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/beetle/bite-small.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\beetle", "bite_small")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/beetle/bite-small2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\beetle", "bite_small2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/beetle/bite-small3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\beetle", "bite_small3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/giant/giant1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\giant", "giant1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/giant/giant2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\giant", "giant2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/giant/giant3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\giant", "giant3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/giant/giant4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\giant", "giant4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/giant/giant5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\giant", "giant5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr10.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr10")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr11.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr11")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr12.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr12")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr13.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr13")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr14.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr14")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr15.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr15")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr6.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr6")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr7.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr7")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr8.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr8")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/gutteral beast/mnstr9.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\gutteral beast", "mnstr9")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/misc/wolfman.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\misc", "wolfman")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/ogre/ogre1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\ogre", "ogre1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/ogre/ogre2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\ogre", "ogre2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/ogre/ogre3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\ogre", "ogre3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/ogre/ogre4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\ogre", "ogre4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/ogre/ogre5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\ogre", "ogre5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade10.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade10")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade11.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade11")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade12.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade12")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade13.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade13")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade14.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade14")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade15.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade15")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade6.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade6")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade7.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade7")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade8.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade8")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/shade/shade9.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\shade", "shade9")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/slime/slime1.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\slime", "slime1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/slime/slime10.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\slime", "slime10")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/slime/slime2.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\slime", "slime2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/slime/slime3.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\slime", "slime3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/slime/slime4.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\slime", "slime4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/slime/slime5.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\slime", "slime5")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/slime/slime6.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\slime", "slime6")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/slime/slime7.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\slime", "slime7")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/slime/slime8.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\slime", "slime8")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/NPC/slime/slime9.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\NPC\slime", "slime9")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/rpg_sound_pack/RPG Sound Pack/world/door.wav"), "/Game/BiomeSurvivor/Audio/RPG/rpg_sound_pack\RPG Sound Pack\world", "door")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/beltHandle1.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "beltHandle1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/beltHandle2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "beltHandle2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/bookClose.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "bookClose")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/bookFlip1.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "bookFlip1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/bookFlip2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "bookFlip2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/bookFlip3.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "bookFlip3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/bookOpen.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "bookOpen")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/bookPlace1.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "bookPlace1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/bookPlace2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "bookPlace2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/bookPlace3.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "bookPlace3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/chop.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "chop")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/cloth1.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "cloth1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/cloth2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "cloth2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/cloth3.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "cloth3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/cloth4.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "cloth4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/clothBelt.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "clothBelt")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/clothBelt2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "clothBelt2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/creak1.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "creak1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/creak2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "creak2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/creak3.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "creak3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/doorClose_1.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "doorClose_1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/doorClose_2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "doorClose_2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/doorClose_3.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "doorClose_3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/doorClose_4.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "doorClose_4")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/doorOpen_1.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "doorOpen_1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/doorOpen_2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "doorOpen_2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/drawKnife1.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "drawKnife1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/drawKnife2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "drawKnife2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/drawKnife3.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "drawKnife3")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/dropLeather.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "dropLeather")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/footstep00.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "footstep00")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/footstep01.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "footstep01")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/footstep02.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "footstep02")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/footstep03.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "footstep03")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/footstep04.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "footstep04")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/footstep05.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "footstep05")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/footstep06.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "footstep06")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/footstep07.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "footstep07")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/footstep08.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "footstep08")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/footstep09.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "footstep09")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/handleCoins.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "handleCoins")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/handleCoins2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "handleCoins2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/handleSmallLeather.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "handleSmallLeather")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/handleSmallLeather2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "handleSmallLeather2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/knifeSlice.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "knifeSlice")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/knifeSlice2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "knifeSlice2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/metalClick.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "metalClick")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/metalLatch.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "metalLatch")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/metalPot1.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "metalPot1")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/metalPot2.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "metalPot2")
    import_audio(os.path.join(project_root, "Assets/Audio/RPG/RPGsounds_Kenney/OGG/metalPot3.ogg"), "/Game/BiomeSurvivor/Audio/RPG/RPGsounds_Kenney\OGG", "metalPot3")

    # --- SFX (100 files) ---
    ensure_directory("/Game/BiomeSurvivor/Audio/SFX")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_air_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_air_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_air_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_air_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_air_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_air_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_door_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_door_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_door_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_door_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_door_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_door_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_door_04.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_door_04")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_door_05.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_door_05")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_footstep_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_footstep_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_footstep_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_footstep_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_footstep_wet_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_footstep_wet_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_footstep_wet_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_footstep_wet_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_footstep_wet_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_footstep_wet_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_footstep_wood_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_footstep_wood_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_footstep_wood_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_footstep_wood_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_footstep_wood_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_footstep_wood_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_footstep_wood_04.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_footstep_wood_04")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_glass_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_glass_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_glass_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_glass_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_glass_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_glass_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_glass_04.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_glass_04")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_glass_05.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_glass_05")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_glass_06.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_glass_06")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_hit_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_hit_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_hit_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_hit_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_hit_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_hit_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_items_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_items_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_items_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_items_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_lock_open_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_lock_open_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_ambient_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_ambient_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_ambient_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_ambient_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_ambient_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_ambient_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_ambient_04.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_ambient_04")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_construction_site.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_construction_site")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_highway.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_highway")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_machine_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_machine_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_machine_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_machine_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_machine_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_machine_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_machine_04.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_machine_04")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_water_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_water_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_water_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_water_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_loop_water_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_loop_water_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_metal_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_metal_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_metal_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_metal_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_metal_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_metal_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_metal_04.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_metal_04")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_metal_05.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_metal_05")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_metal_06.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_metal_06")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_metal_hit_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_metal_hit_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_metal_hit_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_metal_hit_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_04.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_04")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_05.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_05")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_06.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_06")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_07.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_07")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_08.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_08")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_09.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_09")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_10.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_10")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_11.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_11")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_12.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_12")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_13.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_13")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_14.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_14")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_15.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_15")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_16.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_16")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_17.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_17")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_18.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_18")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_19.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_19")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_20.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_20")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_21.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_21")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_22.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_22")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_23.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_23")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_24.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_24")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_25.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_25")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_26.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_26")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_27.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_27")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_28.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_28")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_29.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_29")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_30.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_30")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_31.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_31")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_32.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_32")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_33.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_33")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_34.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_34")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_35.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_35")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_36.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_36")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_misc_37.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_misc_37")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_stones_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_stones_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_stones_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_stones_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_stones_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_stones_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_switch_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_switch_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_switch_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_switch_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_thunder_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_thunder_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_wood_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_wood_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_wood_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_wood_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_wood_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_wood_03")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_wood_04.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_wood_04")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_wood_hit_01.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_wood_hit_01")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_wood_hit_02.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_wood_hit_02")
    import_audio(os.path.join(project_root, "Assets/Audio/SFX/sfx_100_v2/sfx100v2_wood_hit_03.ogg"), "/Game/BiomeSurvivor/Audio/SFX/sfx_100_v2", "sfx100v2_wood_hit_03")

    # --- Weather (3 files) ---
    ensure_directory("/Game/BiomeSurvivor/Audio/Weather")
    import_audio(os.path.join(project_root, "Assets/Audio/Weather/wind/wind/Wind.ogg"), "/Game/BiomeSurvivor/Audio/Weather/wind\wind", "Wind")
    import_audio(os.path.join(project_root, "Assets/Audio/Weather/wind/wind/Wind2.ogg"), "/Game/BiomeSurvivor/Audio/Weather/wind\wind", "Wind2")
    import_audio(os.path.join(project_root, "Assets/Audio/Weather/wind/wind/Wind3.ogg"), "/Game/BiomeSurvivor/Audio/Weather/wind\wind", "Wind3")

    # --- Wildlife (1 files) ---
    ensure_directory("/Game/BiomeSurvivor/Audio/Wildlife")
    import_audio(os.path.join(project_root, "Assets/Audio/Wildlife/birds_ambient.ogg"), "/Game/BiomeSurvivor/Audio/Wildlife", "birds_ambient")


    # ─── Summary ──────────────────────────────────────────────────
    unreal.log("=" * 60)
    unreal.log("BiomeSurvivor Asset Import - Complete!")
    unreal.log("=" * 60)

if __name__ == "__main__":
    main()
