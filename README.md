# TD-Assimp
Assimp 3d model loader C++ SOP for TouchDesigner.
![image](https://user-images.githubusercontent.com/10091486/217917786-7470d41b-0adf-46b0-a49d-c2e87c7a67ba.png)

To test out the SOP, you should be able to pull or download this repo and simply open test.toe.

If you wish to use TD-Assimp in your own projects, be sure to link your C++ SOP to the TDAssimp.dll like so:
![image](https://user-images.githubusercontent.com/10091486/217921641-e44c97d9-fb4e-482a-8834-8952df6d7716.png)

After that, be sure to copy the assimp-vc142-mt.dll file into the same directory as TdAssimp.dll, otherwise the SOP will not work.
![image](https://user-images.githubusercontent.com/10091486/217921798-e49e505c-6927-4725-8f47-8c1e71417c6e.png)


## What is Assimp
Assimp stands for Asset Importer Library. It's an open source library for importing a vast number of file format types. It can export some types as well, but that functionality is not present in this SOP, as it's designed for importing meshes in to TouchDesigner only.

## How does this differ from TouchDesigner's file in SOP?

- Large number of supported 3d import formats
- A number of useful mesh post processing functions

### Supported formats:

TouchDesigner's file in SOP supports the following formats:
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

A thing to note - TD-Assimp only tries to import mesh data, and it will flatten it down to a single mesh - so if you have an FBX file or similar that contains rigged meshes or animated geometries, or separted objets, keep in mind this SOP will not parse things out, it will flatten it down to a single SOP, no groups etc.

## Mesh Post Processing:

TD-Assimp can do a number of really useful mesh [post processing steps](https://assimp.sourceforge.net/lib_html/postprocess_8h.html), making it more optimized or suitable for PBR shading. I have also introduced a google filament specific piece of functionality that calculates and encodes the [TBN matrix as a quaternion](https://github.com/google/filament/blob/main/libs/math/include/math/mat3.h), for smaller vertex attribute size, as well as a [MikkTSpace](http://www.mikktspace.com/) tangent calculation algorithm as an option.

These are the post processing flags implemented for this SOP plugin as custom parameters:

- **Gen Normals if missing (aiProcess_GenNormals)**
  - Generates normals for all faces of all meshes. This is ignored if normals are already there at the time this flag is evaluated. Model importers try to load them from the source file, so they're usually already there. Face normals are shared between all points of a single face, so a single point can have multiple normals, which forces the library to duplicate vertices in some cases.

- **Calc Tangent Space (aiProcess_GenNormals)**
  - Calculates the tangents and bitangents for the imported meshes. Does nothing if a mesh does not have normals. You might want this post processing step to be executed if you plan to use tangent space calculations such as normal mapping applied to the meshes.

- **Tangent Algorithm**
  - Assimp has it's own tangent calculation algorithm which works well, but I've gone ahead and implemented the MikkTSpace algorithm as well. This is a more robust algorithm that is used by many other 3D applications. However it does increase the number of verticies, so try both and see if Assimp version works fine visually.

- **Recalc Bitangent**
  - Recalculates the bitangent using the normal and the tangent.

- **TBN as Quaternion**
  - Encodes the TBN 3x3 matrix as a quaternion. This is useful for packing the data into a single float4. This is data that google filament uses for it's PBR shader. If you are using standard TouchDesigner shaders, you can ignore this and keep it off, as it will add a decent amount of overhead to the Assimp SOP.

- **Join Identical Verticies (aiProcess_JoinIdenticalVertices)**
  - Identifies and joins identical vertex data sets within all imported meshes. After this step is run, each mesh contains unique vertices, so a vertex may be used by multiple faces. You usually want to use this post processing step. If your application deals with indexed geometry, this step is compulsory or you'll just waste rendering time. If this flag is not specified, no vertices are referenced by more than one face and no index buffer is required for rendering.

- **Validate Data Structure (aiProcess_ValidateDataStructure)**
  - Validates the imported scene data structure. This makes sure that all indices are valid, all animations and bones are linked correctly, all material references are correct .. etc. If you want to inspect the Assimp log, you can plug the assimp SOP into an info DAT and split by the character ` to break them into lines.

- **Improve Cache Locality (aiProcess_ImproveCacheLocality)**
  - Reorders triangles for better vertex cache locality. The step tries to improve the ACMR (average post-transform vertex cache miss ratio) for all meshes. The implementation runs in O(n) and is roughly based on the 'tipsify' algorithm. If you intend to render huge models in hardware, this step might be of interest to you.

- **Fix Infacing Normals (aiProcess_FixInfacingNormals)**
  - This step tries to determine which meshes have normal vectors that are facing inwards and inverts them. The algorithm is simple but effective: the bounding box of all vertices + their normals is compared against the volume of the bounding box of all vertices without their normals. This works well for most objects, problems might occur with planar surfaces. However, the step tries to filter such cases. The step inverts all in-facing normals. Generally it is recommended to enable this step, although the result is not always correct.

- **Sort By PType (aiProcess_SortByPType)**
  - This step splits meshes with more than one primitive type in homogeneous sub-meshes. The step is executed after the triangulation step. After the step returns, just one bit is set in aiMesh::mPrimitiveTypes. This is especially useful for real-time rendering where point and line primitives are often ignored or rendered separately. You can use the AI_CONFIG_PP_SBP_REMOVE option to specify which primitive types you need. This can be used to easily exclude lines and points, which are rarely used, from the import.

- **Find Degenerates (aiProcess_FindDegenerates)**
  - This step searches all meshes for degenerate primitives and converts them to proper lines or points. When used with Sort By PType above, this will effectively filter out degenerates as they will be converted to lines or points.

- **Find Invalid Data (aiProcess_FindInvalidData)**
  - This step searches all meshes for invalid data, such as zeroed normal vectors or invalid UV coords and removes/fixes them. This is intended to get rid of some common exporter errors. This is especially useful for normals. If they are invalid, and the step recognizes this, they will be removed and can later be recomputed, i.e. by Gen Normals if missing parameter.
The step will also remove meshes that are infinitely small and reduce animation tracks consisting of hundreds if redundant keys to a single key.

- **Gen UV Coords (aiProcess_GenUVCoords)** 
  - This step converts non-UV mappings (such as spherical or cylindrical mapping) to proper texture coordinate channels.
Most applications will support UV mapping only, so you will probably want to specify this step in every case. Note that Assimp is not always able to match the original mapping implementation of the 3D app which produced a model perfectly. It's always better to let the modelling app compute the UV channels - 3ds max, Maya, Blender, LightWave, and Modo do this for example.

- **Transform UV Coords (aiProcess_TransformUVCoords)**
  - This step applies per-texture UV transformations and bakes them into stand-alone vtexture coordinate channels. UV transformations are specified per-texture. This step processes all textures with transformed input UV coordinates and generates a new (pre-transformed) UV channel which replaces the old channel. Most applications won't support UV transformations, so you will probably want to specify this step.

- **Optimize Meshes (aiProcess_OptimizeMeshes)**
  - A postprocessing step to reduce the number of meshes. This will, in fact, reduce the number of draw calls. This is a very effective optimization and is recommended to be used together with "Optimize Graph" parameter enabled if possible.

- **Optimize Graph (aiProcess_OptimizeGraph)**
  - A postprocessing step to optimize the scene hierarchy. Nodes without animations, bones, lights or cameras assigned are 
  collapsed and joined. Node names can be lost during this step. Use this flag with caution. Most simple files will be collapsed to a single node, so complex hierarchies are usually completely lost. This a very effective optimization if you just want to get the model data, convert it to your own format, and render it as fast as possible which is our goal with TD-Assimp as a SOP. This flag is designed to be used with the Optimize Meshes parameter for best results.

- **Flip Winding Order (aiProcess_FlipWindingOrder)**
  - This step adjusts the output face winding order to be CW.

- **Vertex Color Tint**
  - Simply tints the vertex color from default of white, to a color of your choosing. you can do this with separate SOP's down stream if you wish as well, this is just a slightly faster approach to bundle it in with Assimp on the c++ side.

## Support

If you find this useful, please feel free to support this work on my [Github Sponsors](https://github.com/sponsors/EnviralDesign) OR on [Patreon](https://www.patreon.com/EnviralDesign). If you have aeny issues or bugs, please drop an issue here.
