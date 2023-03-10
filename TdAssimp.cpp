/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "DataAndTypes.h"

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

	DLLEXPORT
	void
	FillSOPPluginInfo(SOP_PluginInfo *info)
	{
		// Always return SOP_CPLUSPLUS_API_VERSION in this function.
		info->apiVersion = SOPCPlusPlusAPIVersion;

		// The opType is the unique name for this TOP. It must start with a 
		// capital A-Z character, and all the following characters must lower case
		// or numbers (a-z, 0-9)
		info->customOPInfo.opType->setString("TdAssimp");

		// The opLabel is the text that will show up in the OP Create Dialog
		info->customOPInfo.opLabel->setString("TD Assimp");

		// Will be turned into a 3 letter icon on the nodes
		info->customOPInfo.opIcon->setString("tda");

		// Information about the author of this OP
		info->customOPInfo.authorName->setString("Lucas Morgan");
		info->customOPInfo.authorEmail->setString("enviraldesign@gmail.com");

		// This SOP works with 0 or 1 inputs
		info->customOPInfo.minInputs = 0;
		info->customOPInfo.maxInputs = 1;

	}

	DLLEXPORT
	SOP_CPlusPlusBase*
	CreateSOPInstance(const OP_NodeInfo* info)
	{
		// Return a new instance of your class every time this is called.
		// It will be called once per SOP that is using the .dll
		return new TdAssimp(info);
	}

	DLLEXPORT
	void
	DestroySOPInstance(SOP_CPlusPlusBase* instance)
	{
		// Delete the instance here, this will be called when
		// Touch is shutting down, when the SOP using that instance is deleted, or
		// if the SOP loads a different DLL
		delete (TdAssimp*)instance;
	}

};


TdAssimp::TdAssimp(const OP_NodeInfo* info) : myNodeInfo(info)
{
	myExecuteCount = 0;
	myOffset = 0.0;
	myChop = "";

	myChopChanName = "";
	myChopChanVal = 0;

	myDat = "N/A";
}

TdAssimp::~TdAssimp()
{

}

void
TdAssimp::getGeneralInfo(SOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = false;

	//if direct to GPU loading:
	// TODO: set this up later when we have basic functionality working for CPU.
	//bool directGPU = inputs->getParInt("Gpudirect") != 0 ? true : false;
	bool directGPU = false;
	ginfo->directToGPU = directGPU;

}


class AssimpLogStream : public Assimp::LogStream {
public:
	// assimp knows to call the write function inside this logstream class, so overriding it here with our own functionality.
	void write(const char* message) {
		myLog.append(message);
		myLog.append("`");
		// using the tilde character as a line break character 
		// since the entire log ends up getting written to a line of the info dat.
	}
};

//-----------------------------------------------------------------------------------------------------
//										Generate a geometry on CPU
//-----------------------------------------------------------------------------------------------------


//std::vector<float> Position_Data; // 3
//std::vector<float> Normal_Data; // 3
//std::vector<float> Tangent_Data; // 4
//std::vector<float> Tangent_Data_MikkT; // 4
//std::vector<float> Bitangent_Data; // 3
//std::vector<float> TbnQuat_Data; // 4
//std::vector<float> Uv_Data; // 2
//int numFaces;

int get_num_faces(const SMikkTSpaceContext* context) {
	Mesh* working_mesh = static_cast<Mesh*> (context->m_pUserData);
	return working_mesh->numTris;
}

int get_num_vertices_of_face(const SMikkTSpaceContext* context, const int iFace) {
	Mesh* working_mesh = static_cast<Mesh*> (context->m_pUserData);
	return working_mesh->vertsPerFace;
}

void get_position(const SMikkTSpaceContext* context, float* outpos, const int iFace, const int iVert) {
	Mesh* working_mesh = static_cast<Mesh*> (context->m_pUserData);

	//const int size = 3; // xyz
	const int prim_offset = (iFace * working_mesh->vertsPerFace);
	const int vtx_offset = (iVert);
	//std::cout << prim_offset + vtx_offset << std::endl;
	//std::cout << working_mesh->Position_Data[prim_offset + vtx_offset].x << std::endl;
	outpos[0] = working_mesh->Position_Data[prim_offset + vtx_offset].x; // x
	outpos[1] = working_mesh->Position_Data[prim_offset + vtx_offset].y; // y
	outpos[2] = working_mesh->Position_Data[prim_offset + vtx_offset].z; // z
}

void get_normal(const SMikkTSpaceContext* context, float* outnormal, const int iFace, const int iVert) {
	Mesh* working_mesh = static_cast<Mesh*> (context->m_pUserData);

	//const int size = 3; // xyz
	const int prim_offset = (iFace * working_mesh->vertsPerFace);
	const int vtx_offset = (iVert);

	outnormal[0] = working_mesh->Normal_Data[prim_offset + vtx_offset].x; // x
	outnormal[1] = working_mesh->Normal_Data[prim_offset + vtx_offset].y; // y
	outnormal[2] = working_mesh->Normal_Data[prim_offset + vtx_offset].z; // z
}

void get_tex_coords(const SMikkTSpaceContext* context, float* outuv, const int iFace, const int iVert) {
	Mesh* working_mesh = static_cast<Mesh*> (context->m_pUserData);

	//const int size = 3; // uvw
	const int prim_offset = (iFace * working_mesh->vertsPerFace);
	const int vtx_offset = (iVert);
	const int offset = prim_offset + vtx_offset;

	//std::cout << iFace << "," << iVert << "," << working_mesh->Uv_Data.size() << std::endl;
	//std::cout << working_mesh->Uv_Data[offset].u << std::endl;
	outuv[0] = working_mesh->Uv_Data[prim_offset + vtx_offset].u; // x
	outuv[1] = working_mesh->Uv_Data[prim_offset + vtx_offset].v; // y
	// w is not neccesary, but we still need to define it to get correct offsets.
}

void set_tspace_basic(const SMikkTSpaceContext* context, const float* tangentu, const float fSign, const int iFace, const int iVert) {
	Mesh* working_mesh = static_cast<Mesh*> (context->m_pUserData);

	const int size = 4; // xyzw
	const int prim_offset = (iFace * working_mesh->vertsPerFace * size);
	const int vtx_offset = (iVert * size);
	//std::cout << tangentu[0] << "," << tangentu[1] << "," << tangentu[2] << "," << fSign << std::endl;
	working_mesh->Tangent_Data[prim_offset + vtx_offset + 0] = tangentu[0]; // x
	working_mesh->Tangent_Data[prim_offset + vtx_offset + 1] = tangentu[1]; // y
	working_mesh->Tangent_Data[prim_offset + vtx_offset + 2] = tangentu[2]; // z

	// NOTE: I have no idea why this fSign value needs to be flipped, that may be a mistake, but it does make the results look correct...
	working_mesh->Tangent_Data[prim_offset + vtx_offset + 3] = -fSign; // w (sign / handedness)
}

void
TdAssimp::execute(SOP_Output* output, const OP_Inputs* inputs, void* reserved)
{
	myExecuteCount++;
	std::cout << "======================================" << std::endl;

	// output style, choose TouchDesigner(0) or Google Filament(1)
	int Attributestyle = inputs->getParInt("Attributestyle");
	//Attributestyle = 1;

	// enable the Tangentalgorithm parameter, maybe able to delete this later due to a bug.
	inputs->enablePar("Tangentalgorithm", 1);

	// determine if we are processing tangents as assimp imported style OR as mikktspace tangents.
	int DoMikktSpaceTangents = inputs->getParInt("Tangentalgorithm") == 1;
	//DoMikktSpaceTangents = 0;
	
	
	// assign the various helper functions to mikktspace's interface object so it knows how to interact with our data.
	iface.m_getNumFaces = get_num_faces;
	iface.m_getNumVerticesOfFace = get_num_vertices_of_face;
	iface.m_getNormal = get_normal;
	iface.m_getPosition = get_position;
	iface.m_getTexCoord = get_tex_coords;
	iface.m_setTSpaceBasic = set_tspace_basic;
	context.m_pInterface = &iface;

	// get the vertex color tint from the custom parameters.
	inputs->getParDouble4("Vertexcolortint", vertexTint[0], vertexTint[1], vertexTint[2], vertexTint[3]);

	/////////////////////////////// LOGGING ///////////////////////////////////
	
	// clear the log at the start of each cook.
	myLog = "";

	// Select the kinds of messages you want to receive on this log stream
	// const unsigned int severity = Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Warn | Assimp::Logger::Err;
	const unsigned int severity = 0
		| (inputs->getParInt("Debugging")	== 1 ? Assimp::Logger::Debugging : 0)
		| (inputs->getParInt("Info")		== 1 ? Assimp::Logger::Info : 0)
		| (inputs->getParInt("Warning")		== 1 ? Assimp::Logger::Warn : 0)
		| (inputs->getParInt("Error")		== 1 ? Assimp::Logger::Err : 0)
	;

	// if at least one of the logging flags are set, we create the logger and attach the log stream to it.
	if(severity > 0){
		// Create a logger instance
		Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
		// attach the log stream function to the logger, so we print messages automatically.
		Assimp::DefaultLogger::get()->attachStream(new AssimpLogStream, severity);
	}

	// if no flags are set, we save ourselves a bit of performance, and destroy the log and replace with a nulllogger.
	else {
		Assimp::DefaultLogger::kill();
	}

	/*
	other ways to use the logger with custom messages.
	Assimp::DefaultLogger::get()->info("This is an info level message.");
	Assimp::DefaultLogger::get()->debug("This is a debug level message.");
	Assimp::DefaultLogger::get()->warn("This is a warn level message.");
	Assimp::DefaultLogger::get()->error("This is a error level message.");
	*/


	/////////////////////////////// LOADING 3D DATA VIA ASSIMP ///////////////////////////////////

	// get the file path from the File parameter.
	const char* pFile = inputs->getParString("File");

	// define/declare an instance of the assimp importer.
	Assimp::Importer importer;

	// read the file while also doing some post processing.
	// post processing documentation: http://assimp.sourceforge.net/lib_html/postprocess_8h.html

	const unsigned int meshProcessingFlags = 0
		| aiProcess_CalcTangentSpace // calc tangent space must be enabled
		| (inputs->getParInt("Joinidenticalvertices")		== 1 ? aiProcess_JoinIdenticalVertices : 0)
		| aiProcess_Triangulate // triangulation must be enabled.
		| aiProcess_GenNormals
		| (inputs->getParInt("Validatedatastructure")		== 1 ? aiProcess_ValidateDataStructure : 0)
		| (inputs->getParInt("Improvecachelocality")		== 1 ? aiProcess_ImproveCacheLocality : 0)
		| (inputs->getParInt("Fixinfacingnormals")			== 1 ? aiProcess_FixInfacingNormals : 0)
		| (inputs->getParInt("Sortbyptype")					== 1 ? aiProcess_SortByPType : 0)
		| (inputs->getParInt("Finddegenerates")				== 1 ? aiProcess_FindDegenerates : 0)
		| (inputs->getParInt("Findinvaliddata")				== 1 ? aiProcess_FindInvalidData : 0)
		| (inputs->getParInt("Genuvcoords")					== 1 ? aiProcess_GenUVCoords : 0)
		| (inputs->getParInt("Transformuvcoords")			== 1 ? aiProcess_TransformUVCoords : 0)
		| (inputs->getParInt("Optimizemeshes")				== 1 ? aiProcess_OptimizeMeshes : 0)
		| (inputs->getParInt("Optimizegraph")				== 1 ? aiProcess_OptimizeGraph : 0)
		| (inputs->getParInt("Flipwindingorder")			== 1 ? aiProcess_FlipWindingOrder : 0)
		| (inputs->getParInt("Flipwindingorder")			== 1 ? aiProcess_FlipWindingOrder : 0)
		;

	// read the file into the scene variable.
	const aiScene* scene = importer.ReadFile( pFile, meshProcessingFlags );

	// If the import failed, report it, and halt the flow.
	if (nullptr == scene) {
		myError = "3D file does not exist or failed to load.";
	}

	// if import succeeded, proceed.
	else {

		Mesh mesh;
		vtxOffset = 0;

		///////////////////////////////////////////////////////////////////////
		////////////////// STANDARD MESH PROCESSING METHOD ////////////////////
		///////////////////////////////////////////////////////////////////////
		
		if (DoMikktSpaceTangents == 0) {

			// for each mesh in the assimp scene.
			for (int mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++) {

				// for each vertex in this mesh.
				for (int i = 0; i < scene->mMeshes[mesh_index]->mNumVertices; i++)
				{
					mesh.FaceIndex_Data.push_back(i);

					// ADD VERTEX POSITIONS
					int HasPositions = scene->mMeshes[mesh_index]->HasPositions();
					mesh.Position_Data.push_back(
						Position(
							HasPositions ? scene->mMeshes[mesh_index]->mVertices[i][0] : 0, // x
							HasPositions ? scene->mMeshes[mesh_index]->mVertices[i][1] : 0, // y
							HasPositions ? scene->mMeshes[mesh_index]->mVertices[i][2] : 0  // z
						)
					);

					// ADD VERTEX COLORS
					int HasVertexColors = scene->mMeshes[mesh_index]->HasVertexColors(0);
					mesh.Color_Data.push_back(
						Color(
							HasVertexColors ? scene->mMeshes[mesh_index]->mColors[0][i][0] * vertexTint[0] : (float)vertexTint[0], // r
							HasVertexColors ? scene->mMeshes[mesh_index]->mColors[0][i][1] * vertexTint[1] : (float)vertexTint[1], // g
							HasVertexColors ? scene->mMeshes[mesh_index]->mColors[0][i][2] * vertexTint[2] : (float)vertexTint[2], // b
							HasVertexColors ? scene->mMeshes[mesh_index]->mColors[0][i][3] * vertexTint[3] : (float)vertexTint[3]  // a
						)
					);

					// ADD UVS
					// get the number of texture layers for this particular object.
					// NOTE: as of TouchDesigner 2021.16410 adding multiple uv sets is bugged, but this will be fixed in future versions.
					// at that point we can attempt to re introduce multiple uv sets support, but does anyone even need this?
					int numTextureLayers = scene->mMeshes[mesh_index]->GetNumUVChannels();
					numTextureLayers = std::min(1, numTextureLayers);
					//numTextureLayers = 1;
					mesh.Uv_Data.push_back(
						TexCoord(
							numTextureLayers ? scene->mMeshes[mesh_index]->mTextureCoords[0][i][0] : 0, // u
							numTextureLayers ? scene->mMeshes[mesh_index]->mTextureCoords[0][i][1] : 0, // v
							numTextureLayers ? scene->mMeshes[mesh_index]->mTextureCoords[0][i][2] : 0   // w
						)
					);

					// ADD NORMALS
					int HasNormals = scene->mMeshes[mesh_index]->HasNormals();
					mesh.Normal_Data.push_back(
						Vector(
							HasNormals ? scene->mMeshes[mesh_index]->mNormals[i][0] : 0, // x
							HasNormals ? scene->mMeshes[mesh_index]->mNormals[i][1] : 0, // y
							HasNormals ? scene->mMeshes[mesh_index]->mNormals[i][2] : 0  // z
						)
					);
					normal[0] = scene->mMeshes[mesh_index]->mNormals[i][0];
					normal[1] = scene->mMeshes[mesh_index]->mNormals[i][1];
					normal[2] = scene->mMeshes[mesh_index]->mNormals[i][2];

					// ADD TANGENT / BITANGENT
					int HasTangentsAndBitangents = scene->mMeshes[mesh_index]->HasTangentsAndBitangents();
					mesh.Tangent_Data.push_back(HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][0] : 0); // x
					mesh.Tangent_Data.push_back(HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][1] : 0); // y
					mesh.Tangent_Data.push_back(HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][2] : 0); // z
					mesh.Tangent_Data.push_back(1.0); // handedness / sign. 1 is assumed, since assimp's internally matches openGL and also they do not provide this value in their data structure.

					tangent[0] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][0] : 0;
					tangent[1] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][1] : 0;
					tangent[2] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][2] : 0;
					tangentSign = 1.0; // handedness / sign. 1 is assumed, since assimp's internally matches openGL and also they do not provide this value in their data structure.
					
					if (Attributestyle == 1) {
						// recalc bitangent
						cross(normal, tangent, bitangent);
					}
					else {
						bitangent[0] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mBitangents[i][0] : 0; // x
						bitangent[1] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mBitangents[i][1] : 0; // y
						bitangent[2] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mBitangents[i][2] : 0; // z
					}

					mesh.Bitangent_Data.push_back(bitangent[0]); // x
					mesh.Bitangent_Data.push_back(bitangent[1]); // y
					mesh.Bitangent_Data.push_back(bitangent[2]); // z
					
					if (Attributestyle == 1) {
						tbn_to_quat(
							tangent[0], tangent[1], tangent[2], tangentSign,
							bitangent[0], bitangent[1], bitangent[2],
							normal[0], normal[1], normal[2], tbnquat
						);

						mesh.TbnQuat_Data.push_back(tbnquat[0]);
						mesh.TbnQuat_Data.push_back(tbnquat[1]);
						mesh.TbnQuat_Data.push_back(tbnquat[2]);
						mesh.TbnQuat_Data.push_back(tbnquat[3]);

					}


					vtxOffset += 1;
				} // end of for loop for verts.

				// update number of tris after each mesh.
				mesh.numTris += scene->mMeshes[mesh_index]->mNumFaces;

			} // end of for loop for meshes.
		}
		///////////////////////////////////////////////////////////////////////
		//////////////////// MIKKT MESH PROCESSING METHOD /////////////////////
		///////////////////////////////////////////////////////////////////////
		else {

			vtxOffset = 0;

			// for each mesh in the assimp scene.
			for (int mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++) {

				// for each face in this mesh.
				for (int face_index = 0; face_index < scene->mMeshes[mesh_index]->mNumFaces; face_index++)
				{
					// for each vertex in this face.
					for (int vertex_index = 0; vertex_index < scene->mMeshes[mesh_index]->mFaces[face_index].mNumIndices; vertex_index++)
					{
						int i = scene->mMeshes[mesh_index]->mFaces[face_index].mIndices[vertex_index];

						std::cout << i << std::endl;
						// ADD VERTEX POSITIONS
						int HasPositions = scene->mMeshes[mesh_index]->HasPositions();
						mesh.Position_Data.push_back(
							Position(
								HasPositions ? scene->mMeshes[mesh_index]->mVertices[i][0] : 0, // x
								HasPositions ? scene->mMeshes[mesh_index]->mVertices[i][1] : 0, // y
								HasPositions ? scene->mMeshes[mesh_index]->mVertices[i][2] : 0  // z
							)
						);


						// ADD VERTEX COLORS
						int HasVertexColors = scene->mMeshes[mesh_index]->HasVertexColors(0);
						mesh.Color_Data.push_back(
							Color(
								HasVertexColors ? scene->mMeshes[mesh_index]->mColors[0][i][0] * vertexTint[0] : (float)vertexTint[0], // r
								HasVertexColors ? scene->mMeshes[mesh_index]->mColors[0][i][1] * vertexTint[1] : (float)vertexTint[1], // g
								HasVertexColors ? scene->mMeshes[mesh_index]->mColors[0][i][2] * vertexTint[2] : (float)vertexTint[2], // b
								HasVertexColors ? scene->mMeshes[mesh_index]->mColors[0][i][3] * vertexTint[3] : (float)vertexTint[3]  // a
							)
						);

						// ADD UVS
						// get the number of texture layers for this particular object.
						// NOTE: as of TouchDesigner 2021.16410 adding multiple uv sets is bugged, but this will be fixed in future versions.
						// at that point we can attempt to re introduce multiple uv sets support, but does anyone even need this?
						int numTextureLayers = scene->mMeshes[mesh_index]->GetNumUVChannels();
						numTextureLayers = std::min(1, numTextureLayers);
						mesh.Uv_Data.push_back(
							TexCoord(
								numTextureLayers ? scene->mMeshes[mesh_index]->mTextureCoords[0][i][0] : 0, // u
								numTextureLayers ? scene->mMeshes[mesh_index]->mTextureCoords[0][i][1] : 0, // v
								numTextureLayers ? scene->mMeshes[mesh_index]->mTextureCoords[0][i][2] : 0   // w
							)
						);

						// ADD NORMALS
						int HasNormals = scene->mMeshes[mesh_index]->HasNormals();
						mesh.Normal_Data.push_back(
							Vector(
								HasNormals ? scene->mMeshes[mesh_index]->mNormals[i][0] : 0, // x
								HasNormals ? scene->mMeshes[mesh_index]->mNormals[i][1] : 0, // y
								HasNormals ? scene->mMeshes[mesh_index]->mNormals[i][2] : 0  // z
							)
						);

						// ADD TANGENT / BITANGENT
						int HasTangentsAndBitangents = scene->mMeshes[mesh_index]->HasTangentsAndBitangents();
						mesh.Tangent_Data.push_back(HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][0] : 0); // x
						mesh.Tangent_Data.push_back(HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][1] : 0); // y
						mesh.Tangent_Data.push_back(HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][2] : 0); // z
						mesh.Tangent_Data.push_back(1.0); // handedness / sign. 1 is assumed, since assimp's internally matches openGL and also they do not provide this value in their data structure.

						normal[0] = scene->mMeshes[mesh_index]->mNormals[i][0];
						normal[1] = scene->mMeshes[mesh_index]->mNormals[i][1];
						normal[2] = scene->mMeshes[mesh_index]->mNormals[i][2];
						tangent[0] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][0] : 0;
						tangent[1] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][1] : 0;
						tangent[2] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mTangents[i][2] : 0;

						if (Attributestyle == 1) {
							// recalc bitangent, writes data to third argument.
							cross(normal, tangent, bitangent);
						}
						else {
							bitangent[0] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mBitangents[i][0] : 0; // x
							bitangent[1] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mBitangents[i][1] : 0; // y
							bitangent[2] = HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mBitangents[i][2] : 0; // z
						}

						mesh.Bitangent_Data.push_back(bitangent[0]); // x
						mesh.Bitangent_Data.push_back(bitangent[1]); // y
						mesh.Bitangent_Data.push_back(bitangent[2]); // z

					} // for each vertex in this face.

					mesh.FaceIndex_Data.push_back(vtxOffset + 0);
					mesh.FaceIndex_Data.push_back(vtxOffset + 1);
					mesh.FaceIndex_Data.push_back(vtxOffset + 2);
					vtxOffset += 3;

					mesh.numTris += 1;

				} // for each face in this mesh.

			} // for each mesh in the assimp scene.

			// do mikktspace generation of new tangent data. 
			// tangent data will be written into the mesh object, updating old values.
			context.m_pUserData = &mesh;
			genTangSpaceDefault(&context);
			//genTangSpace(&context, 10); // alternate if we care about setting smoothing angle argument.

			// since mikktspace tangent generation happens as a full post process after our mesh data is fully assembled
			// we could not calculate tbn quat until after that step, so we loop back through our mesh data now that
			// mikkt has been updated there, and calculate tbnquat from final data.
			for (int vertex_index = 0; vertex_index < mesh.Position_Data.size(); vertex_index++) {
				
				normal[0] = mesh.Normal_Data[vertex_index].x;
				normal[1] = mesh.Normal_Data[vertex_index].y;
				normal[2] = mesh.Normal_Data[vertex_index].z;

				tangent[0] = mesh.Tangent_Data[(vertex_index * 4) +0];
				tangent[1] = mesh.Tangent_Data[(vertex_index * 4) +1];
				tangent[2] = mesh.Tangent_Data[(vertex_index * 4) +2];
				tangentSign = mesh.Tangent_Data[(vertex_index * 4) +3];

				bitangent[0] = mesh.Bitangent_Data[(vertex_index * 3) +0];
				bitangent[1] = mesh.Bitangent_Data[(vertex_index * 3) +1];
				bitangent[2] = mesh.Bitangent_Data[(vertex_index * 3) +2];

				if (Attributestyle == 1) {
					
					tbn_to_quat(
						tangent[0], tangent[1], tangent[2], tangentSign,
						bitangent[0], bitangent[1], bitangent[2],
						normal[0], normal[1], normal[2], tbnquat
					);
					

					// frisvadTangentSpace( tangent, bitangent, normal, tbnquat );


					mesh.TbnQuat_Data.push_back(tbnquat[0]);
					mesh.TbnQuat_Data.push_back(tbnquat[1]);
					mesh.TbnQuat_Data.push_back(tbnquat[2]);
					mesh.TbnQuat_Data.push_back(tbnquat[3]);

				}

			}

		}



		////// TODO: maybe eventually get some tangent smoothing in here?
		//for (int tri_index = 0; tri_index < mesh.numTris; tri_index++) {
		//
		//}

		//Triangle tri;
		//tri.uvs[0].u = 0;
		//tri.uvs[0].u = 1;

		//Assimp::DefaultLogger::get()->info("VERTS AFTER WELD: " + std::to_string(mesh.numTris));
		

		/*
		////////////////////// DO MESH WELDING /////////////////////////
		if (DoMikktSpaceTangents == 1) {

			std::vector<float> vertex_data_in;
			int num_verts_pre_weld = mesh.Position_Data.size();
			int num_floats_per_vert = 0; // just initializing here.

			// Attributestyle == 0, TouchDesigner = 17 floats (P[3] + N[3] + Cd[4] + uv[3] + T[4] )
			// Attributestyle == 0, TouchDesigner = 14 floats (P[3] + N[3] + Cd[4] + uv[3] )
			// Attributestyle == 0, TouchDesigner = 3 floats (P[3] )
			if (Attributestyle == 0) {
				num_floats_per_vert = 13;
			}

			// Attributestyle == 1, GoogleFilament = 13 floats (P[3] + mesh_color[4] + mesh_uv0[2] + mesh_tangents[4] )
			else if (Attributestyle == 1) {
				num_floats_per_vert = 13;
			}

			// initialize some destination memory.
			std::vector<int> remap_table (num_verts_pre_weld, 0);
			std::vector<float> vertex_data_out(num_verts_pre_weld * num_floats_per_vert, 0);

			// assemble vertex data in the structure that is required for the Attributestyle:

			if (Attributestyle == 0) { // TouchDesigner
				
				for (int i = 0; i < mesh.Position_Data.size(); i++) { // i is vertex index.
					
					vertex_data_in.push_back(mesh.Position_Data[i].x);
					vertex_data_in.push_back(mesh.Position_Data[i].y);
					vertex_data_in.push_back(mesh.Position_Data[i].z);

					vertex_data_in.push_back(mesh.Normal_Data[i].x);
					vertex_data_in.push_back(mesh.Normal_Data[i].y);
					vertex_data_in.push_back(mesh.Normal_Data[i].z);

					vertex_data_in.push_back(mesh.Color_Data[i].r);
					vertex_data_in.push_back(mesh.Color_Data[i].g);
					vertex_data_in.push_back(mesh.Color_Data[i].b);
					vertex_data_in.push_back(mesh.Color_Data[i].a);

					vertex_data_in.push_back(mesh.Uv_Data[i].u);
					vertex_data_in.push_back(mesh.Uv_Data[i].v);
					vertex_data_in.push_back(mesh.Uv_Data[i].w);

					//vertex_data_in.push_back(mesh.Tangent_Data[i * 3 + 0]);
					//vertex_data_in.push_back(mesh.Tangent_Data[i * 3 + 1]);
					//vertex_data_in.push_back(mesh.Tangent_Data[i * 3 + 2]);

				}
			}

			int num_verts_post_weld = WeldMesh(remap_table.data() , vertex_data_out.data() , vertex_data_in.data() , num_verts_pre_weld, num_floats_per_vert);

			Assimp::DefaultLogger::get()->info("VERTS AFTER WELD: " + std::to_string(num_verts_post_weld));
			Assimp::DefaultLogger::get()->info("VERTS AFTER WELD: " 
				+ std::to_string(remap_table[0]) + ',' 
				+ std::to_string(remap_table[1]) + ','
			);

		}

		/////////////////////// END MESH WELDING ////////////////////////
		*/


		if (Attributestyle == 0) { // IF ATTRIBUTE STYLE IS TouchDesigner:

			// add positions, normals, and colors.
			output->addPoints(mesh.Position_Data.data(), vtxOffset);
			output->setNormals(mesh.Normal_Data.data(), vtxOffset, 0);
			output->setColors(mesh.Color_Data.data(), vtxOffset, 0);
			
			// add uvs, setTexCoords() seem broken, so we can't add them in one go.
			int texindex = 0;
			for (TexCoord i : mesh.Uv_Data) {
				output->setTexCoord(&i, 1, texindex);
				texindex++;
			}

			// add tangents.
			SOP_CustomAttribData Tangents_Attribute("T", 4, AttribType::Float);
			Tangents_Attribute.floatData = mesh.Tangent_Data.data();
			output->setCustomAttribute(&Tangents_Attribute, output->getNumPoints());

		}

		if (Attributestyle == 1) { // IF ATTRIBUTE STYLE IS GoogleFilament:

			// add positions, TD requires this at a bare minimum. Filament looks for a vec4 called mesh_position though.
			output->addPoints(mesh.Position_Data.data(), vtxOffset);

			// add normals, this is extra attributes to upload to GPU, but it gives the SOP correct shading in TD. maybe we turn this off later.
			output->setNormals(mesh.Normal_Data.data(), vtxOffset, 0);

			// add mesh_position, the vertex attribute filament actually looks for.
			// since our position data is vec3, we expand it here to vec4.
			SOP_CustomAttribData mesh_position_attrs("mesh_position", 4, AttribType::Float);
			//expandedPositions.clear();
			for (int i = 0; i < mesh.Position_Data.size(); i++) {
				expandedPositions.push_back(mesh.Position_Data[i].x);
				expandedPositions.push_back(mesh.Position_Data[i].y);
				expandedPositions.push_back(mesh.Position_Data[i].z);
				expandedPositions.push_back(1.0f);}
			// assign it as a custom attribute, even though it's a fairly standard one by filament's standards.
			mesh_position_attrs.floatData = expandedPositions.data();
			output->setCustomAttribute(&mesh_position_attrs, output->getNumPoints());
			//Assimp::DefaultLogger::get()->info("mesh.Position_Data.size(): " + std::to_string(mesh.Position_Data.size()));
			//Assimp::DefaultLogger::get()->info("output->getNumPoints(): " + std::to_string(output->getNumPoints()));
			//Assimp::DefaultLogger::get()->info("expandedPositions.size(): " + std::to_string(expandedPositions.size()));
			
			// add mesh_color for filament. fortunately color data is already a vec4.
			// unfortunately can't assign it directly for c++ reasons. this is probably unefficient, so lets look at it later.
			// maybe we can not use TD's Color class to store this in general.
			SOP_CustomAttribData mesh_color_attrs("mesh_color", 4, AttribType::Float);
			//expandedColors.clear();
			for (int i = 0; i < mesh.Color_Data.size(); i++) {
				expandedColors.push_back(mesh.Color_Data[i].r);
				expandedColors.push_back(mesh.Color_Data[i].g);
				expandedColors.push_back(mesh.Color_Data[i].b);
				expandedColors.push_back(mesh.Color_Data[i].a);}
			mesh_color_attrs.floatData = expandedColors.data();
			output->setCustomAttribute(&mesh_color_attrs, output->getNumPoints());
			
			// set mesh_uv0 for filament.
			SOP_CustomAttribData mesh_uv0_attrs("mesh_uv0", 2, AttribType::Float);
			//expandedUvs0.clear();
			int texindex = 0;
			for (TexCoord i : mesh.Uv_Data) {
				// output->setTexCoord(&i, 1, texindex);
				expandedUvs0.push_back(*(&i.u));
				expandedUvs0.push_back(*(&i.v));
				texindex++;
			}
			mesh_uv0_attrs.floatData = expandedUvs0.data();
			output->setCustomAttribute(&mesh_uv0_attrs, output->getNumPoints());
		
			// set mesh_tangents for filament.
			SOP_CustomAttribData mesh_tangents_attrs("mesh_tangents", 4, AttribType::Float);
			mesh_tangents_attrs.floatData = mesh.TbnQuat_Data.data();
			output->setCustomAttribute(&mesh_tangents_attrs, output->getNumPoints());

			// debugging output
			SOP_CustomAttribData mesh_debugging_attrs("mesh_debugging", 4, AttribType::Float);
			mesh_debugging_attrs.floatData = debugging.data();
			output->setCustomAttribute(&mesh_debugging_attrs, output->getNumPoints());
		
		}


		////////////////////////////////////////////////
		/////////////////// STANDARD MESH TRIANGLES ////
		////////////////////////////////////////////////
		if (DoMikktSpaceTangents == 0) {
			triOffset = 0;
			for (int mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++) {

				if (scene->mMeshes[mesh_index]->HasFaces()) {
					for (int i = 0; i < scene->mMeshes[mesh_index]->mNumFaces; i++)
					{
						
						output->addTriangle(
							scene->mMeshes[mesh_index]->mFaces[i].mIndices[0] + triOffset,
							scene->mMeshes[mesh_index]->mFaces[i].mIndices[1] + triOffset,
							scene->mMeshes[mesh_index]->mFaces[i].mIndices[2] + triOffset
						);
					}
					triOffset += scene->mMeshes[mesh_index]->mNumVertices;
				}
			}
		}

		else {

			////////////////////////////////////////////////
			/////////////////// MIKKT MESH TRIANGLES ///////
			////////////////////////////////////////////////
			triOffset = 0;
			for (int face_index = 0; face_index < mesh.numTris; face_index++) {

				output->addTriangle(
					mesh.FaceIndex_Data[(face_index * 3) + 0],
					mesh.FaceIndex_Data[(face_index * 3) + 1],
					mesh.FaceIndex_Data[(face_index * 3) + 2]
				);

			}

		}

		mesh.Position_Data.clear();
		mesh.Normal_Data.clear();
		mesh.Uv_Data.clear();
		mesh.Color_Data.clear();
		mesh.Tangent_Data.clear();
		mesh.Bitangent_Data.clear();
		mesh.TbnQuat_Data.clear();
		mesh.FaceIndex_Data.clear();

		expandedUvs0.clear();
		expandedColors.clear();
		expandedPositions.clear();

		debugging.clear();

	}

}




void
TdAssimp::executeVBO(SOP_VBOOutput* output,const OP_Inputs* inputs,void* reserved)
{
	myExecuteCount++;

	if (!output)
	{
		return;
	}

}

//-----------------------------------------------------------------------------------------------------
//								CHOP, DAT, and custom parameters
//-----------------------------------------------------------------------------------------------------

void
TdAssimp::getErrorString(OP_String * error, void*)
{
	error->setString(myError.c_str());
	myError.clear();
}

int32_t
TdAssimp::getNumInfoCHOPChans(void* reserved)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send 4 channels.
	return 4;
}

void
TdAssimp::getInfoCHOPChan(int32_t index,
								OP_InfoCHOPChan* chan, void* reserved)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)myOffset;
	}

	if (index == 2)
	{
		chan->name->setString(myChop.c_str());
		chan->value = (float)myOffset;
	}

	if (index == 3)
	{
		chan->name->setString(myChopChanName.c_str());
		chan->value = myChopChanVal;
	}
}

bool
TdAssimp::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved)
{
	infoSize->rows = 1;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
TdAssimp::getInfoDATEntries(int32_t index,
								int32_t nEntries,
								OP_InfoDATEntries* entries,
								void* reserved)
{
	char tempBuffer[4096];


	if (index == 0)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "log");
#else // macOS
		strlcpy(tempBuffer, "log", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		strcpy_s(tempBuffer, myLog.c_str());
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), myLog.c_str());
#endif
		//std::cout << myLog.c_str() << std::endl;
		entries->values[1]->setString(tempBuffer);
	}

	/*
	if (index == 1)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "offset");
#else // macOS
		strlcpy(tempBuffer, "offset", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%g", myOffset);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 2)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "DAT input name");
#else // macOS
		strlcpy(tempBuffer, "offset", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		strcpy_s(tempBuffer, myDat.c_str());
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(tempBuffer);
	}
	*/
}


void
TdAssimp::setupParameters(OP_ParameterManager* manager, void* reserved)
{
	
	/////////////////////////////////// IMPORT PAGE /////////////////////////////////////////
	// FILE
	{
		OP_StringParameter	p;

		p.name = "File";
		p.label = "3D File";
		p.page = "Import";

		OP_ParAppendResult res = manager->appendFile(p);
		assert(res == OP_ParAppendResult::Success);
	}


	/////////////////////////////////// POST PROCESSING PAGE /////////////////////////////////////////
	
	///// leaving this commented out now, since we can assume user will always want tangents, if they aren't in the model lets calc them.
	///// simplifies the whole array of parameters greatly into a situation where there are no parameter dependancy, thus less confusing.
	/*

		// Gen Normals - generates normals only for points that do not already have any.
	{
		OP_NumericParameter p;

		p.name = "Gennormals";
		p.label = "Gen Normals (if missing)";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}


	// Calc Tangent Space
	{
		OP_NumericParameter p;

		p.name = "Calctangentspace";
		p.label = "Calc Tangent Space";
		p.page = "Processing";
		p.defaultValues[0] = true;


		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	// Tangent Algorithm
	{
		OP_StringParameter p;
		p.name = "Tangentalgorithm";
		p.label = "Tangent Algorithm";
		p.page = "Processing";
		p.defaultValue = "Assimp";
		std::array<const char*, 4> Names =
		{
			"Assimp",
			"Mikktspace"
		};
		std::array<const char*, 4> Labels =
		{
			"Assimp",
			"Mikktspace"
		};
		OP_ParAppendResult res = manager->appendMenu(p, int(Names.size()), Names.data(), Labels.data());

		assert(res == OP_ParAppendResult::Success);
	}
	
	///////// we are ditching these two parameters in favor of a more intuitive single parameter, called "Attribute Style" below. 
	///////// realistically, there is no big benefit to users to have bitangents in a TD shading world, but it is relevant for filament.
	///////// further more, bitangent in filament is not consumed directly, it's packed into a quat so we can reduce this to 1 switch.
	/*
	// Recalculatebitangent
	{
		OP_NumericParameter p;

		p.name = "Recalculatebitangent";
		p.label = "Recalc Bitangent";
		p.page = "Processing";
		p.defaultValues[0] = true;


		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	//////////////
	// TBN as Quat
	//////////////
	{
		OP_NumericParameter p;

		p.name = "Tbnasquat";
		p.label = "TBN as Quaternion";
		p.page = "Processing";
		p.defaultValues[0] = false;


		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	// Join Identical Vertices
	{
		OP_NumericParameter p;

		p.name = "Joinidenticalvertices";
		p.label = "Join Identical Vertices";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	/* since TD is openGL.. there shouldn't be a need for this ever, but leaving incase we find an edge case.
	// Make Left Handed
	{
		OP_NumericParameter p;

		p.name = "Makelefthanded";
		p.label = "Make Left Handed";
		p.page = "Processing";
		p.defaultValues[0] = false;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	/* CURRENTLY DISABLED. c++ SOP can only create triangles, so this option is meaningless for now.
	// Triangulate - since the c++ sop cannot create quads or ngons, this is probably going to need to be on. 
	// However it can add cook time, so leaving it as a toggle incase user already has incoming mesh as triangles, and wants to save a bit of cook time.
	{
		OP_NumericParameter p;

		p.name = "Triangulate";
		p.label = "Triangulate";
		p.page = "Processing";
		p.defaultValues[0] = false;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	/* leaving this commented out for now, because for use it requires additional flags to to be set to define which components are to be removed. TODO.
	// Remove Componnet http://assimp.sourceforge.net/lib_html/postprocess_8h.html
	{
		OP_NumericParameter p;

		p.name = "Removecomponent";
		p.label = "Remove Component";
		p.page = "Processing";
		p.defaultValues[0] = false;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	/* requires a smoothing angle to be defined, also this cannot be used with the above Gen Normals, so leaving it disabled for now. TODO: http://assimp.sourceforge.net/lib_html/postprocess_8h.html
	// Gen Smooth Normals
	{
		OP_NumericParameter p;

		p.name = "Gensmoothnormals";
		p.label = "Gen Smooth Normals";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	/* disabling this one for now, needs additional info supplied, and also what are good values for number of tris? does this mean anything since we get all our geo in our sop anyways?
	// Split Large Meshes
	{
		OP_NumericParameter p;

		p.name = "Splitlargemeshes";
		p.label = "Split Large Meshes";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	/* seems uneecessary.. can in some rare cases cause problems: http://assimp.sourceforge.net/lib_html/postprocess_8h.html
	// Pre Transform Vertices
	{
		OP_NumericParameter p;

		p.name = "Pretransformvertices";
		p.label = "Pre Transform Vertices";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	/* we'll eventually need this as an optimization probably when we support animated meshes. we're a ways off from that, so leaving this disabled.
	// Limit Bone Weights
	{
		OP_NumericParameter p;

		p.name = "Limitboneweights";
		p.label = "Limit Bone Weights";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	
	// Validate Data Structure
	{
		OP_NumericParameter p;

		p.name = "Validatedatastructure";
		p.label = "Validate Data Structure";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	// Improve Cache Locality
	{
		OP_NumericParameter p;

		p.name = "Improvecachelocality";
		p.label = "Improve Cache Locality";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	/* since we're not pulling material data from meshes loaded with assimp (yet?) we'll just leave this disabled for now.
	// Remove Redundant Materials
	{
		OP_NumericParameter p;

		p.name = "Removeredundantmaterials";
		p.label = "Remove Redundant Materials";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	// Fix Infacing Normals
	{
		OP_NumericParameter p;

		p.name = "Fixinfacingnormals";
		p.label = "Fix Infacing Normals";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	// Sort By PType
	{
		OP_NumericParameter p;

		p.name = "Sortbyptype";
		p.label = "Sort By PType";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	// Find Degenerates
	{
		OP_NumericParameter p;

		p.name = "Finddegenerates";
		p.label = "Find Degenerates";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	// Find Invalid Data
	{
		OP_NumericParameter p;

		p.name = "Findinvaliddata";
		p.label = "Find Invalid Data";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	// Gen UV Coords
	{
		OP_NumericParameter p;

		p.name = "Genuvcoords";
		p.label = "Gen UV Coords";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	// Transform UV Coords
	{
		OP_NumericParameter p;

		p.name = "Transformuvcoords";
		p.label = "Transform UV Coords";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	/* not currently needed since we do not have plans atm to have the imported sop generate instancing data.
	// Find Instances
	{
		OP_NumericParameter p;

		p.name = "Findfnstances";
		p.label = "Find Instances";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	// Optimize Meshes
	{
		OP_NumericParameter p;

		p.name = "Optimizemeshes";
		p.label = "Optimize Meshes";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	// Optimize Graph
	{
		OP_NumericParameter p;

		p.name = "Optimizegraph";
		p.label = "Optimize Graph";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	/* this will likely never be desired, since TD is open GL.. 
	// Flip UVs
	{
		OP_NumericParameter p;

		p.name = "Flipuvs";
		p.label = "Flip UVs";
		p.page = "Processing";
		p.defaultValues[0] = false;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	// Flip Winding Order
	{
		OP_NumericParameter p;

		p.name = "Flipwindingorder";
		p.label = "Flip Winding Order";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	/* revisit this when bones/skinning is implemented.
	// Split By Bone Count
	{
		OP_NumericParameter p;

		p.name = "Splitbybonecount";
		p.label = "Split By Bone Count";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	/* revisit this when bones / skinning is implemented.
	// Debone
	{
		OP_NumericParameter p;

		p.name = "Debone";
		p.label = "Debone";
		p.page = "Processing";
		p.defaultValues[0] = true;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	*/

	// Output Attributes Style
	{
		OP_StringParameter p;
		p.name = "Attributestyle";
		p.label = "Attribute Style";
		p.page = "Sop Output";
		p.defaultValue = "Touchdesigner";
		std::array<const char*, 4> Names =
		{
			"Touchdesigner",
			"Filament"
		};
		std::array<const char*, 4> Labels =
		{
			"TouchDesigner",
			"Filament"
		};
		OP_ParAppendResult res = manager->appendMenu(p, int(Names.size()), Names.data(), Labels.data());

		assert(res == OP_ParAppendResult::Success);
	}

	{
		OP_NumericParameter p;
		p.name = "Vertexcolortint";
		p.label = "Vertex Color Tint";
		p.page = "Sop Output";

		const int ArraySize = 4;

		const std::array<double, ArraySize>  DefaultValues = { 1.0, 1.0, 1.0, 1.0 };
		const std::array<double, ArraySize>  MinSliders = { 0.0, 0.0, 0.0, 0.0 };
		const std::array<double, ArraySize>  MaxSliders = { 1.0, 1.0, 1.0, 1.0 };
		const std::array<double, ArraySize>  MinValues = { 0.0, 0.0, 0.0, 0.0 };
		const std::array<double, ArraySize>  MaxValues = { 1.0, 1.0, 1.0, 1.0 };
		const std::array<bool, ArraySize>  ClampMins = { true, true, true, true };
		const std::array<bool, ArraySize>  ClampMaxes = { true, true, true, true };
		for (int i = 0; i < DefaultValues.size(); ++i)
		{
			p.defaultValues[i] = DefaultValues[i];
			p.minSliders[i] = MinSliders[i];
			p.maxSliders[i] = MaxSliders[i];
			p.minValues[i] = MinValues[i];
			p.maxValues[i] = MaxValues[i];
			p.clampMins[i] = ClampMins[i];
			p.clampMaxes[i] = ClampMaxes[i];
		}
		OP_ParAppendResult res = manager->appendRGBA(p);

		assert(res == OP_ParAppendResult::Success);
	}

	/////////////////////////////////// LOGGING PAGE /////////////////////////////////////////
	// Debugging
	{
		OP_NumericParameter p;

		p.name = "Debugging";
		p.label = "Debugging";
		p.page = "Logging";
		p.defaultValues[0] = false;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	// Info
	{
		OP_NumericParameter p;

		p.name = "Info";
		p.label = "Info";
		p.page = "Logging";
		p.defaultValues[0] = false;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	// Warning
	{
		OP_NumericParameter p;

		p.name = "Warning";
		p.label = "Warning";
		p.page = "Logging";
		p.defaultValues[0] = false;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}

	// Error
	{
		OP_NumericParameter p;

		p.name = "Error";
		p.label = "Error";
		p.page = "Logging";
		p.defaultValues[0] = false;

		OP_ParAppendResult res = manager->appendToggle(p);
		assert(res == OP_ParAppendResult::Success);
	}
	
	/*
	// CHOP
	{
		OP_StringParameter	np;

		np.name = "Chop";
		np.label = "CHOP";

		OP_ParAppendResult res = manager->appendCHOP(np);
		assert(res == OP_ParAppendResult::Success);
	}

	
	// scale
	{
		OP_NumericParameter	np;

		np.name = "Scale";
		np.label = "Scale";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = -10.0;
		np.maxSliders[0] = 10.0;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// shape
	{
		OP_StringParameter	sp;

		sp.name = "Shape";
		sp.label = "Shape";

		sp.defaultValue = "Cube";

		const char *names[] = { "Cube", "Triangle", "Line" };
		const char *labels[] = { "Cube", "Triangle", "Line" };

		OP_ParAppendResult res = manager->appendMenu(sp, 3, names, labels);
		assert(res == OP_ParAppendResult::Success);
	}

	// GPU Direct
	{
		OP_NumericParameter np;

		np.name = "Gpudirect";
		np.label = "GPU Direct";

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";

		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

	*/

	Assimp::DefaultLogger::kill();

}

void
TdAssimp::pulsePressed(const char* name, void* reserved)
{
	if (!strcmp(name, "Reset"))
	{
		myOffset = 0.0;
	}
}

