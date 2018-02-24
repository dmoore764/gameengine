#pragma once
#include "mesh.h"

enum object3d_type
{
	OBJECT3D_BONE_MODEL,
	OBJECT3D_BASIC_GEOMETRY,
};

enum object3d_physics_behavior
{
	PHYS_NONE,
	PHYS_STATIC,		//non-moving
	PHYS_DYNAMIC,		//movement controlled by physics engine
	PHYS_KINETIC,		//movement controlled by game
};

struct object3d
{
	object3d_type type;
	v3 position;
	quat rotation;
	m4 modelMat;
	union
	{
		bone_model_instance boneModelInstance;
		//basic_mesh geometry;
	};
	texture *tex0;
};

void Object3DUpdate(object3d *obj, float deltaT);
void Object3DCalculateModelMat(object3d *obj);
void Object3DSetModelMatToBone(object3d *obj, object3d *target, const char *boneName);
