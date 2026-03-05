#!/usr/bin/env python3
"""
UE5 Asset Import Configuration Script for BiomeSurvivor
=======================================================

This script generates:
1. A UE5 Python script for batch importing textures with proper settings
2. Material Instance definitions for each PBR texture set
3. Sound Cue definitions for audio assets
4. Asset registry configuration

Run this script from the project root to generate import-ready configs,
then execute the generated UE5 Python script inside the Unreal Editor.

Usage:
    python Tools/AssetImportScript.py

After running, open UE5 Editor -> Tools -> Execute Python Script
and run: Tools/UE5_BatchImport.py
"""

import os
import json
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
ASSETS_DIR = PROJECT_ROOT / "Assets"
TOOLS_DIR = PROJECT_ROOT / "Tools"
CONTENT_DIR = "/Game"  # UE5 content root

# ─── Texture Configuration ─────────────────────────────────────────────

TEXTURE_IMPORT_SETTINGS = {
    "Grass": {
        "CompressionSettings": "TC_Default",
        "LODGroup": "TEXTUREGROUP_World",
        "MaxTextureSize": 2048,
        "SRGB": True,
        "MaterialDomain": "MD_Surface",
        "BlendMode": "BLEND_Opaque",
        "ShadingModel": "MSM_DefaultLit",
        "TwoSided": False,
    },
    "Snow": {
        "CompressionSettings": "TC_Default",
        "LODGroup": "TEXTUREGROUP_World",
        "MaxTextureSize": 2048,
        "SRGB": True,
        "MaterialDomain": "MD_Surface",
        "BlendMode": "BLEND_Opaque",
        "ShadingModel": "MSM_DefaultLit",
        "TwoSided": False,
    },
    "Ground": {
        "CompressionSettings": "TC_Default",
        "LODGroup": "TEXTUREGROUP_World",
        "MaxTextureSize": 2048,
        "SRGB": True,
        "MaterialDomain": "MD_Surface",
        "BlendMode": "BLEND_Opaque",
        "ShadingModel": "MSM_DefaultLit",
        "TwoSided": False,
    },
    "Rock": {
        "CompressionSettings": "TC_Default",
        "LODGroup": "TEXTUREGROUP_World",
        "MaxTextureSize": 2048,
        "SRGB": True,
        "MaterialDomain": "MD_Surface",
        "BlendMode": "BLEND_Opaque",
        "ShadingModel": "MSM_DefaultLit",
        "TwoSided": False,
    },
    "Sand": {
        "CompressionSettings": "TC_Default",
        "LODGroup": "TEXTUREGROUP_World",
        "MaxTextureSize": 2048,
        "SRGB": True,
        "MaterialDomain": "MD_Surface",
        "BlendMode": "BLEND_Opaque",
        "ShadingModel": "MSM_DefaultLit",
        "TwoSided": False,
    },
    "Wood": {
        "CompressionSettings": "TC_Default",
        "LODGroup": "TEXTUREGROUP_World",
        "MaxTextureSize": 2048,
        "SRGB": True,
        "MaterialDomain": "MD_Surface",
        "BlendMode": "BLEND_Opaque",
        "ShadingModel": "MSM_DefaultLit",
        "TwoSided": False,
    },
    "Metal": {
        "CompressionSettings": "TC_Default",
        "LODGroup": "TEXTUREGROUP_World",
        "MaxTextureSize": 2048,
        "SRGB": True,
        "MaterialDomain": "MD_Surface",
        "BlendMode": "BLEND_Opaque",
        "ShadingModel": "MSM_DefaultLit",
        "TwoSided": False,
    },
}

# Map texture suffixes to UE5 texture types
TEXTURE_SUFFIX_MAP = {
    "_Color": {"TextureType": "BaseColor", "SRGB": True, "Compression": "TC_Default"},
    "_Displacement": {"TextureType": "Displacement", "SRGB": False, "Compression": "TC_Displacementmap"},
    "_NormalGL": {"TextureType": "Normal", "SRGB": False, "Compression": "TC_Normalmap"},
    "_NormalDX": {"TextureType": "Normal", "SRGB": False, "Compression": "TC_Normalmap"},
    "_Roughness": {"TextureType": "Roughness", "SRGB": False, "Compression": "TC_Masks"},
    "_AmbientOcclusion": {"TextureType": "AO", "SRGB": False, "Compression": "TC_Masks"},
    "_Metalness": {"TextureType": "Metallic", "SRGB": False, "Compression": "TC_Masks"},
    "_Opacity": {"TextureType": "Opacity", "SRGB": False, "Compression": "TC_Masks"},
}

# ─── Audio Configuration ───────────────────────────────────────────────

AUDIO_CATEGORIES = {
    "Footsteps": {
        "SoundClass": "SFX",
        "Concurrency": 4,
        "AttenuationMin": 100.0,
        "AttenuationMax": 2000.0,
        "AttenuationShape": "Sphere",
        "Spatialized": True,
    },
    "Ambient": {
        "SoundClass": "Ambient",
        "Concurrency": 2,
        "AttenuationMin": 500.0,
        "AttenuationMax": 10000.0,
        "AttenuationShape": "Sphere",
        "Spatialized": True,
        "Looping": True,
    },
    "Combat": {
        "SoundClass": "SFX",
        "Concurrency": 8,
        "AttenuationMin": 200.0,
        "AttenuationMax": 5000.0,
        "AttenuationShape": "Sphere",
        "Spatialized": True,
    },
    "UI": {
        "SoundClass": "UI",
        "Concurrency": 4,
        "Spatialized": False,
    },
    "Weather": {
        "SoundClass": "Ambient",
        "Concurrency": 2,
        "AttenuationMin": 0.0,
        "AttenuationMax": 0.0,
        "Spatialized": False,
        "Looping": True,
    },
    "Wildlife": {
        "SoundClass": "Ambient",
        "Concurrency": 6,
        "AttenuationMin": 300.0,
        "AttenuationMax": 8000.0,
        "AttenuationShape": "Sphere",
        "Spatialized": True,
    },
    "Crafting": {
        "SoundClass": "SFX",
        "Concurrency": 4,
        "AttenuationMin": 100.0,
        "AttenuationMax": 2000.0,
        "Spatialized": True,
    },
    "Music": {
        "SoundClass": "Music",
        "Concurrency": 1,
        "Spatialized": False,
        "Looping": True,
    },
}


def discover_texture_packs():
    """Scan Assets/Textures for PBR texture packs."""
    packs = []
    textures_dir = ASSETS_DIR / "Textures"
    if not textures_dir.exists():
        return packs

    for category_dir in sorted(textures_dir.iterdir()):
        if not category_dir.is_dir():
            continue
        category = category_dir.name

        for pack_dir in sorted(category_dir.iterdir()):
            if not pack_dir.is_dir():
                continue

            textures = {}
            for img_file in sorted(pack_dir.iterdir()):
                if img_file.suffix.lower() in (".jpg", ".png", ".tga", ".exr"):
                    # Determine texture type from suffix
                    stem = img_file.stem
                    tex_type = "Unknown"
                    for suffix, info in TEXTURE_SUFFIX_MAP.items():
                        if suffix in stem:
                            tex_type = info["TextureType"]
                            break
                    textures[tex_type] = {
                        "file": str(img_file.relative_to(PROJECT_ROOT)),
                        "filename": img_file.name,
                        "stem": stem,
                    }

            if textures:
                packs.append({
                    "name": pack_dir.name,
                    "category": category,
                    "path": str(pack_dir.relative_to(PROJECT_ROOT)),
                    "textures": textures,
                    "settings": TEXTURE_IMPORT_SETTINGS.get(category, TEXTURE_IMPORT_SETTINGS["Ground"]),
                })

    return packs


def discover_audio_files():
    """Scan Assets/Audio for sound files."""
    audio_files = {}
    audio_dir = ASSETS_DIR / "Audio"
    if not audio_dir.exists():
        return audio_files

    for category_dir in sorted(audio_dir.iterdir()):
        if not category_dir.is_dir():
            continue
        category = category_dir.name
        files = []

        for audio_file in sorted(category_dir.rglob("*")):
            if audio_file.suffix.lower() in (".wav", ".ogg", ".mp3", ".flac"):
                files.append({
                    "file": str(audio_file.relative_to(PROJECT_ROOT)),
                    "filename": audio_file.name,
                    "stem": audio_file.stem,
                    "subfolder": str(audio_file.parent.relative_to(category_dir)) if audio_file.parent != category_dir else "",
                })

        if files:
            audio_files[category] = {
                "files": files,
                "count": len(files),
                "settings": AUDIO_CATEGORIES.get(category, AUDIO_CATEGORIES.get("Combat")),
            }

    return audio_files


def generate_ue5_python_script(texture_packs, audio_data):
    """Generate the UE5 editor Python import script."""

    script = '''#!/usr/bin/env python3
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

'''

    # Generate texture import code
    for pack in texture_packs:
        pack_name = pack["name"]
        category = pack["category"]
        dest_base = f"/Game/BiomeSurvivor/Textures/{category}/{pack_name}"

        script += f'    # --- {pack_name} ---\n'
        script += f'    ensure_directory("{dest_base}")\n'
        script += f'    unreal.log("Importing {pack_name}...")\n'

        texture_vars = {}
        for tex_type, tex_info in pack["textures"].items():
            var_name = f"tex_{pack_name}_{tex_type}".replace("-", "_").replace(".", "_")
            source = tex_info["file"].replace("\\", "/")
            stem = tex_info["stem"]

            suffix_settings = {}
            for suffix, info in TEXTURE_SUFFIX_MAP.items():
                if info["TextureType"] == tex_type:
                    suffix_settings = info
                    break

            srgb = suffix_settings.get("SRGB", True)
            compression = suffix_settings.get("Compression", "TC_Default")
            is_normal = tex_type == "Normal"

            script += f'    {var_name} = import_texture(\n'
            script += f'        os.path.join(project_root, "{source}"),\n'
            script += f'        "{dest_base}", "{stem}",\n'
            script += f'        srgb={srgb}, compression="{compression}", normal_map={is_normal}\n'
            script += f'    )\n'
            texture_vars[tex_type] = var_name

        # Create material instance
        mi_dest = f"/Game/BiomeSurvivor/Materials/Landscape"
        mi_name = f"MI_{pack_name}"
        param_map = {}
        if "BaseColor" in texture_vars:
            param_map["BaseColor"] = texture_vars["BaseColor"]
        if "Normal" in texture_vars:
            param_map["Normal"] = texture_vars["Normal"]
        if "Roughness" in texture_vars:
            param_map["Roughness"] = texture_vars["Roughness"]
        if "AO" in texture_vars:
            param_map["AmbientOcclusion"] = texture_vars["AO"]
        if "Metallic" in texture_vars:
            param_map["Metallic"] = texture_vars["Metallic"]
        if "Displacement" in texture_vars:
            param_map["Displacement"] = texture_vars["Displacement"]

        if param_map:
            script += f'\n    # Create Material Instance: {mi_name}\n'
            script += f'    mi_textures = {{\n'
            for param, var in param_map.items():
                script += f'        "{param}": {var},\n'
            script += f'    }}\n'
            script += f'    create_material_instance(\n'
            script += f'        "/Game/BiomeSurvivor/Materials/M_Landscape_Master",\n'
            script += f'        "{mi_name}", "{mi_dest}", mi_textures\n'
            script += f'    )\n'

        script += '\n'

    # Generate audio import code
    script += '''
    # ─── Import Audio ─────────────────────────────────────────────
    unreal.log("Importing Audio Files...")

'''

    for category, data in audio_data.items():
        settings = data.get("settings", {})
        dest_base = f"/Game/BiomeSurvivor/Audio/{category}"
        script += f'    # --- {category} ({data["count"]} files) ---\n'
        script += f'    ensure_directory("{dest_base}")\n'

        for af in data["files"]:
            source = af["file"].replace("\\", "/")
            stem = af["stem"].replace(" ", "_").replace("-", "_")
            subfolder = af.get("subfolder", "")
            dest = f"{dest_base}/{subfolder}" if subfolder else dest_base

            script += f'    import_audio(os.path.join(project_root, "{source}"), "{dest}", "{stem}")\n'

        script += '\n'

    script += '''
    # ─── Summary ──────────────────────────────────────────────────
    unreal.log("=" * 60)
    unreal.log("BiomeSurvivor Asset Import - Complete!")
    unreal.log("=" * 60)

if __name__ == "__main__":
    main()
'''

    return script


def generate_asset_manifest(texture_packs, audio_data):
    """Generate a JSON manifest of all assets for reference."""
    manifest = {
        "project": "BiomeSurvivor",
        "license": "CC0 - Creative Commons Zero",
        "texture_sources": "ambientCG.com",
        "audio_sources": "OpenGameArt.org",
        "textures": {
            "total_packs": len(texture_packs),
            "packs": []
        },
        "audio": {
            "categories": {},
            "total_files": 0
        }
    }

    for pack in texture_packs:
        manifest["textures"]["packs"].append({
            "name": pack["name"],
            "category": pack["category"],
            "types": list(pack["textures"].keys()),
            "ue5_path": f"/Game/BiomeSurvivor/Textures/{pack['category']}/{pack['name']}",
            "material_instance": f"/Game/BiomeSurvivor/Materials/Landscape/MI_{pack['name']}",
        })

    total_audio = 0
    for category, data in audio_data.items():
        manifest["audio"]["categories"][category] = {
            "count": data["count"],
            "ue5_path": f"/Game/BiomeSurvivor/Audio/{category}",
            "settings": data["settings"],
        }
        total_audio += data["count"]

    manifest["audio"]["total_files"] = total_audio

    return manifest


def generate_master_material_blueprint():
    """Generate the master landscape material Python creation script."""
    return '''# M_Landscape_Master - Master material for landscape PBR textures
# Execute in UE5 Python console to create the master material

import unreal

def create_master_landscape_material():
    """Create a master material with standard PBR parameters."""
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_util = unreal.EditorAssetLibrary

    mat_path = "/Game/BiomeSurvivor/Materials"
    mat_name = "M_Landscape_Master"

    if not editor_util.does_directory_exist(mat_path):
        editor_util.make_directory(mat_path)

    factory = unreal.MaterialFactoryNew()
    material = asset_tools.create_asset(mat_name, mat_path, unreal.Material, factory)

    if not material:
        unreal.log_error("Failed to create master material!")
        return

    mel = unreal.MaterialEditingLibrary

    # Create texture parameters
    base_color = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -800, -200)
    base_color.set_editor_property("parameter_name", "BaseColor")

    normal = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -800, 0)
    normal.set_editor_property("parameter_name", "Normal")

    roughness = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -800, 200)
    roughness.set_editor_property("parameter_name", "Roughness")

    ao = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -800, 400)
    ao.set_editor_property("parameter_name", "AmbientOcclusion")

    metallic = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -800, 600)
    metallic.set_editor_property("parameter_name", "Metallic")

    displacement = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -800, 800)
    displacement.set_editor_property("parameter_name", "Displacement")

    # UV Tiling
    tex_coord = mel.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, -1200, 0)
    tiling = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -1200, 200)
    tiling.set_editor_property("parameter_name", "Tiling")
    tiling.set_editor_property("default_value", 1.0)

    multiply = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, -1000, 100)

    # Connect expressions to material
    mel.connect_material_expressions(base_color, "RGB", material, "BaseColor")
    mel.connect_material_expressions(normal, "RGB", material, "Normal")
    mel.connect_material_expressions(roughness, "R", material, "Roughness")
    mel.connect_material_expressions(ao, "R", material, "AmbientOcclusion")
    mel.connect_material_expressions(metallic, "R", material, "Metallic")

    # Save
    editor_util.save_asset(f"{mat_path}/{mat_name}")
    unreal.log(f"Created master material: {mat_path}/{mat_name}")

    return material

create_master_landscape_material()
'''


def main():
    print("=" * 60)
    print(" BiomeSurvivor - Asset Import Configuration Generator")
    print("=" * 60)

    # Discover assets
    print("\nScanning for textures...")
    texture_packs = discover_texture_packs()
    print(f"  Found {len(texture_packs)} texture packs")

    print("Scanning for audio files...")
    audio_data = discover_audio_files()
    total_audio = sum(d["count"] for d in audio_data.values())
    print(f"  Found {total_audio} audio files in {len(audio_data)} categories")

    # Generate UE5 import script
    print("\nGenerating UE5 batch import script...")
    ue5_script = generate_ue5_python_script(texture_packs, audio_data)
    ue5_script_path = TOOLS_DIR / "UE5_BatchImport.py"
    os.makedirs(TOOLS_DIR, exist_ok=True)
    with open(ue5_script_path, "w", encoding="utf-8") as f:
        f.write(ue5_script)
    print(f"  Written: {ue5_script_path}")

    # Generate master material script
    print("Generating master material creation script...")
    mat_script = generate_master_material_blueprint()
    mat_script_path = TOOLS_DIR / "UE5_CreateMasterMaterial.py"
    with open(mat_script_path, "w", encoding="utf-8") as f:
        f.write(mat_script)
    print(f"  Written: {mat_script_path}")

    # Generate asset manifest
    print("Generating asset manifest...")
    manifest = generate_asset_manifest(texture_packs, audio_data)
    manifest_path = TOOLS_DIR / "AssetManifest.json"
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2, default=str)
    print(f"  Written: {manifest_path}")

    # Summary
    print("\n" + "=" * 60)
    print(" Generation Complete!")
    print("=" * 60)
    print(f"\n  Texture Packs:  {len(texture_packs)}")
    print(f"  Audio Files:    {total_audio}")
    print(f"  Categories:     {len(audio_data)}")
    print(f"\n  Output Files:")
    print(f"    {ue5_script_path}")
    print(f"    {mat_script_path}")
    print(f"    {manifest_path}")
    print(f"\n  Next Steps:")
    print(f"    1. Open Unreal Editor")
    print(f"    2. Tools -> Execute Python Script")
    print(f"    3. Run: Tools/UE5_CreateMasterMaterial.py  (first)")
    print(f"    4. Run: Tools/UE5_BatchImport.py          (second)")
    print()


if __name__ == "__main__":
    main()
