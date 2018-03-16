#pragma once
#include "mesh.h"
#include "memory_arena.h"
#include "hash.h"
#include "opengl.h"

struct assets
{
	bone_model worm;
	basic_mesh manipulatorX;
	basic_mesh manipulatorY;
	basic_mesh manipulatorZ;
	basic_mesh sphere;
	basic_mesh xaxis;
	basic_mesh yaxis;
	basic_mesh zaxis;
	basic_mesh physShapeRectangularPrism;
	basic_mesh physShapeSphere;
	basic_mesh physShapeCylinder;
	basic_mesh physShapeCone;
	basic_mesh physShapeCapsuleBottom;
	basic_mesh physShapeCapsuleMiddle;
	basic_mesh physShapeCapsuleTop;
	basic_mesh physShapePlane;
	basic_mesh objmarker;

	generic_hash boneModelHash;
	generic_hash basicMeshHash;

	shader basicMeshColoredVerts;
	shader hiddenMeshColoredVerts;
	shader texturedTriangles;
	shader basicMeshSolidColor;
	shader outlineDrawer;
	shader boneModel;

	texture uvTex;
};

void assLoad(assets *ass, memory_arena *arena);
