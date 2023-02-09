# TD-Assimp
Assimp 3d model loader C++ SOP for TouchDesigner.

## What is Assimp
Assimp stands for Asset Importer Library. It's an open source library for importing a vast number of file format types, and can export many types too.

## How does this differ from TouchDesigner's file in SOP?

### Supported formats:

TouchDesigner's file in SOP only supports the following formats:
- TouchDesigner : .tog
- Houdini : .classic, .bhclassic
- Common : .obj

Assimp supports importing the following formats:
- 3D Manufacturing Format (.3mf)
- Collada (.dae, .xml)
- Blender (.blend)
- Biovision BVH (.bvh)
- 3D Studio Max 3DS (.3ds)
- 3D Studio Max ASE (.ase)
- glTF (.glTF)
- glTF2.0 (.glTF)
- KHR_lights_punctual ( 5.0 )
- KHR_materials_pbrSpecularGlossiness ( 5.0 )
- KHR_materials_unlit ( 5.0 )
- KHR_texture_transform ( 5.1 under test )
- FBX-Format, as ASCII and binary (.fbx)
- Stanford Polygon Library (.ply)
- AutoCAD DXF (.dxf)
- IFC-STEP (.ifc)
- Neutral File Format (.nff)
- Sense8 WorldToolkit (.nff)
- Valve Model (.smd, .vta)
- Quake I (.mdl)
- Quake II (.md2)
- Quake III (.md3)
- Quake 3 BSP (.pk3)
- RtCW (.mdc)
- Doom 3 (.md5mesh, .md5anim, .md5camera)
- DirectX X (.x)
- Quick3D (.q3o, .q3s)
- Raw Triangles (.raw)
- AC3D (.ac, .ac3d)
- Stereolithography (.stl)
- Autodesk DXF (.dxf)
- Irrlicht Mesh (.irrmesh, .xml)
- Irrlicht Scene (.irr, .xml)
- Object File Format ( .off )
- Wavefront Object (.obj)
- Terragen Terrain ( .ter )
- 3D GameStudio Model ( .mdl )
- 3D GameStudio Terrain ( .hmp )
- Ogre ( .mesh.xml, .skeleton.xml, .material )
- OpenGEX-Fomat (.ogex)
- Milkshape 3D ( .ms3d )
- LightWave Model ( .lwo )
- LightWave Scene ( .lws )
- Modo Model ( .lxo )
- CharacterStudio Motion ( .csm )
- Stanford Ply ( .ply )
- TrueSpace (.cob, .scn)
- XGL-3D-Format (.xgl)

### Mesh Post Processing:

http://sir-kimmi.de/assimp/lib_html/common.html
