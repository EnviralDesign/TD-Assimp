
/*
#include "DataAndTypes.h"


void mesh_standard(Mesh& mesh, const aiScene* scene) {

	int vtxOffset = 0;

	// for each mesh in the assimp scene.
	for (int mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++) {

		// for each vertex in this mesh.
		for (int i = 0; i < scene->mMeshes[mesh_index]->mNumVertices; i++)
		{

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
			mesh.Bitangent_Data.push_back(HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mBitangents[i][0] : 0); // x
			mesh.Bitangent_Data.push_back(HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mBitangents[i][1] : 0); // y
			mesh.Bitangent_Data.push_back(HasTangentsAndBitangents ? scene->mMeshes[mesh_index]->mBitangents[i][2] : 0); // z

			// for now doing a wasteful recalculation of the bitangents... think assimp is mucking this up but not 100% sure yet.
			// eventually can probably make the call to nix bitangents all together if the ones from assimp are not superior to 
			// calculating on the fly or TD etc.
			normal[0] = scene->mMeshes[mesh_index]->mNormals[i][0];
			normal[1] = scene->mMeshes[mesh_index]->mNormals[i][1];
			normal[2] = scene->mMeshes[mesh_index]->mNormals[i][2];
			tangent[0] = scene->mMeshes[mesh_index]->mTangents[i][0];
			tangent[1] = scene->mMeshes[mesh_index]->mTangents[i][1];
			tangent[2] = scene->mMeshes[mesh_index]->mTangents[i][2];
			crossProduct(normal, tangent, bitangent);
			mesh.Bitangent_Recalc_Data.push_back(HasTangentsAndBitangents ? bitangent[0] : 0); // x;
			mesh.Bitangent_Recalc_Data.push_back(HasTangentsAndBitangents ? bitangent[1] : 0); // y;
			mesh.Bitangent_Recalc_Data.push_back(HasTangentsAndBitangents ? bitangent[2] : 0); // z;

			vtxOffset += 1;
		} // end of for loop for verts.

		// update number of tris after each mesh.
		mesh.numTris += scene->mMeshes[mesh_index]->mNumFaces;

	} // end of for loop for meshes.

}

*/

