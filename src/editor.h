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
	float rotationAmount;
	v3 initialUp;
	v3 initialRight;
	v3 initialNormal;
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

struct editor
{
	int numObjects;
	obj3d_editor objects[1000];
	obj3d_editor *edit;

	float snapToGrid;
	float snapToAngle;
	int editState;
	int localHandle;
	manipulator objHandle;
	rotator objRotator;

	v3 camPt;
	float camTheta;
	float camPhi;
	v3 camUp;
	float camZoom;
	v3 camPos;

	float vfov;
	bool mouseWasDown;
	bool showDebug;

	bool escWasDown;
	bool gWasDown; //set global transform
	bool lWasDown; //set local transform
	bool sWasDown; //set scale mode
	bool tWasDown; //set translate mode
	bool rWasDown; //set rotate mode
	bool xWasDown; //x rotation axis
	bool yWasDown; //y rotation axis
	bool zWasDown; //z rotation axis
};

void editorInit(editor *);
void editorNewObj3D(editor *e);
void editorMoveCamera(editor *e, float *scrollVelX, float *scrollVelY);
void editorUpdateCamera(editor *e);
