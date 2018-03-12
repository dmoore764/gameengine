#pragma once
#include "game_math.h"
#include "opengl.h"

enum class manipulator_states
{
	GRABBABLE,
	IS_MANIPULATING,
};

struct manipulator
{
	manipulator_states state;

	union
	{
		line_seg_2d axisHandle[3];
		struct
		{
			line_seg_2d xAxisHandle;
			line_seg_2d yAxisHandle;
			line_seg_2d zAxisHandle;
		};
	};

	int whichAxis;
	line_seg_3d axisLine;
	v3 holdingOffset;
};

enum class rotator_states
{
	PICK_AXIS,
	AXIS_PICKED,
	IS_ROTATING,
};

struct rotator
{
	rotator_states state;

	plane rotatePlane;

	int whichAxis;
	quat initialRotation;	//rotation of object when rotation is initiated
	float rotationOffset;	//angle of point on plane that is acting as the rotation handle
	v3 initialUp;
	v3 initialRight;
	v3 initialNormal;

	vertex_array_object *axisToDraw;
	vertex_array_object *axisPlaneToDraw;
};

struct obj3d_mesh_assignment
{
	float validDist; //0 standard
	char name[64];
};

struct obj3d_editor
{
	bool editing;

	bool startsActive;
	bool startsVisible;
	
	int objType;
	int physType;

	v3 position;
	quat rotation;
	v3 scale;
	m4 modelMat;
	vertex_array_object *curVao;

	int numLODs;
	obj3d_mesh_assignment lods[5];

	char initialAnimation[64];

	int shape;
	float mass;
	float restitution;
	v3 dimensions;
	v3 offset;
};
