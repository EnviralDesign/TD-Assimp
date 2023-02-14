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
#include <cmath>

#include <mikktspace.h>
#include <mikktspace.c>

#include <weldmesh.h>

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

std::vector<float> expandedPositions;
std::vector<float> expandedColors;
std::vector<float> expandedUvs0;

std::vector<float> debugging;

class Mesh {
public:
	std::vector<Position> Position_Data; // 3
	std::vector<Vector> Normal_Data; // 3
	std::vector<TexCoord> Uv_Data; // 3
	std::vector<Color> Color_Data; // 4
	std::vector<float> Tangent_Data; // 4
	std::vector<float> Bitangent_Data; // 3
	std::vector<float> TbnQuat_Data; // 4
	std::vector<int32_t> FaceIndex_Data; // 4
	int numTris = 0; // init'd here, but updated in main for loop.
	int vertsPerFace = 3; // always 3 , always using triangles for our implementation.
};

struct Vertex {
	float x, y, z;
};

struct UV {
	float u, v;
};

struct Triangle {
	Vertex vertices[3];
	UV uvs[3];
};

struct Tangent {
	float x, y, z;
};

/*
std::vector<Tangent> ComputeTangents(const std::vector<Triangle>& triangles) {
    std::vector<Tangent> tangents;
    tangents.resize(triangles.size() * 3);

    for (int i = 0; i < triangles.size(); i++) {
        // Get triangle vertices and UVs
        const Vertex& v0 = triangles[i].vertices[0];
        const Vertex& v1 = triangles[i].vertices[1];
        const Vertex& v2 = triangles[i].vertices[2];

        const UV& uv0 = triangles[i].uvs[0];
        const UV& uv1 = triangles[i].uvs[1];
        const UV& uv2 = triangles[i].uvs[2];

        // Compute edge vectors
        float x1 = v1.x - v0.x;
        float y1 = v1.y - v0.y;
        float z1 = v1.z - v0.z;

        float x2 = v2.x - v0.x;
        float y2 = v2.y - v0.y;
        float z2 = v2.z - v0.z;

        // Compute delta UVs
        float du1 = uv1.u - uv0.u;
        float dv1 = uv1.v - uv0.v;
        float du2 = uv2.u - uv0.u;
        float dv2 = uv2.v - uv0.v;

        // Compute tangent
        float f = 1.0f / (du1 * dv2 - dv1 * du2);

        float tx = f * (dv2 * x1 - dv1 * x2);
        float ty = f * (dv2 * y1 - dv1 * y2);
        float tz = f * (dv2 * z1 - dv1 * z2);

        // Normalize tangent
        float invLen = 1.0f / sqrt(tx * tx + ty * ty + tz * tz);
        tx *= invLen;
        ty *= invLen;
        tz *= invLen;

        // Store tangent for each vertex of the triangle
        tangents[i * 3 + 0] = { tx, ty, tz };
        tangents[i * 3 + 1] = { tx, ty, tz };
        tangents[i * 3 + 2] = { tx, ty, tz };
    }

    return tangents;
}
*/