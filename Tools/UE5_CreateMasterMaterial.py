# M_Landscape_Master - Master material for landscape PBR textures
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
