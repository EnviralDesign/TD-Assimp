#pragma once

#include "TdAssimp.h"
#include "mymath.h"
#include "Mesh_Process.cpp"

#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <array>

#include <mikktspace.h>
#include <mikktspace.c>

// holder for generic data.
Position pos;
Vector vec;
Color col;
TexCoord tex;
std::string	myError;
std::array<double, 4> vertexTint;
float normal[3];
float tangent[3];
float tangentSign;
float bitangent[3];
float tbnquat[4];
int vtxOffset;
int triOffset;

// public variable to append log data to.
std::string myLog;

SMikkTSpaceInterface iface{};
SMikkTSpaceContext context{};

/*
class Vertex {
public:
	Position pos;
	Vector nrm;
	Color col;
	TexCoord tex;
	float tangent[4];
	float bitangent[3];
};
*/

class Mesh {
public:
	std::vector<Position> Position_Data; // 3
	std::vector<Vector> Normal_Data; // 3
	std::vector<TexCoord> Uv_Data; // 3
	std::vector<Color> Color_Data; // 4
	std::vector<float> Tangent_Data; // 4
	//std::vector<float> Tangent_MikkT_Data; // 4
	std::vector<float> Bitangent_Data; // 3
	//std::vector<float> Bitangent_Recalc_Data; // 3
	std::vector<float> TbnQuat_Data; // 4
	std::vector<int> FaceIndex_Data; // 4
	int numTris = 0; // init'd here, but updated in main for loop.
	int vertsPerFace = 3; // always 3 , always using triangles for our implementation.
};