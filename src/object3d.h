#pragma once
#include "mesh.h"
#include "physics.h"

enum object3d_flags
{
	O3D_IN_USE  = 0x1,		//is this a valid object (among the list of objects?)
	O3D_ACTIVE  = 0x2,		//is the object currently being simulated
	O3D_VISIBLE = 0x4,		//if active, is it even visible?
	O3D_CULLED  = 0x8,		//if visible, is it outside drawing range?
};

enum class object3d_types : uint8_t
{
	NON_VISUAL = 0,
	BONE_MODEL = 1,
	BASIC_GEOMETRY = 2,
};

enum class object3d_physics : uint8_t
{
	NONE = 0,
	DYNAMIC = 1,		//movement controlled by physics engine (mass of 0 means non-moving)
	KINETIC = 2,		//movement controlled by game
};

struct object3d
{
	uint8_t flags;
	object3d_types type;
	object3d_physics physicsType;
	v3 position;
	quat rotation;
	m4 modelMat;
	object_physics physics;
	union
	{
		bone_model_instance boneModelInstance;
		basic_mesh basicMesh;
	};
};

void o3dUpdate(object3d *obj, float deltaT);
void o3dCalculateModelMat(object3d *obj);
void o3dSetModelMatToBone(object3d *obj, object3d *target, const char *boneName);
void o3dSetDynamicShape(object3d *obj, physics_shapes shape, float mass, float restitution, v3 dimensions, v3 offset);
void o3dRemovePhysics(object3d *obj);

