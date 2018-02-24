#pragma once
#include "memory_arena.h"
#include "game_math.h"
#include "xml.h"

struct collada_triangle
{
	float pos[3];
	float norm[3];
	float uv[2];
	float color[4];
};

enum collada_source_array_type
{
	COLLADA_FLOAT_ARRAY,
	COLLADA_NAME_ARRAY,
};

struct collada_param
{
	char *name;
	char *type;
};

struct collada_source_array
{
	bool isValid;
	char *id;
	int count;
	collada_source_array_type type;
	union
	{
		float *values;
		char *name_values;
	};
	int stride;

	int numParams;
	collada_param params[10];
};

struct collada_poly_semantic
{
	bool isValid;
	char *semantic;
	int source_index;
	int offset;
	int set; //for multiple uv coords or colors
};

struct collada_vertex
{
	int position_index; //source assumed to be 0
	int normal_index;	//source assumed to be 1
	int map_index[4];
	int color_index[4];
};

struct collada_poly
{
	collada_vertex vert[3];
};

struct collada_geometry
{
	char *name;
	char *id;
	int numSources;
	collada_source_array sources[10];

	int numSemantics;
	collada_poly_semantic semantics[10];

	int vertexAttributeCount;

	int numPolys;
	collada_poly *polys;
};

struct collada_animation
{
	char *id;
	int numSources;
	collada_source_array *sources;

	char *target;
};

struct collada_joint_weight
{
	int joint_index;
	float weight;
};

struct collada_vertex_weight
{
	int numWeights;
	collada_joint_weight *weights;
};

struct collada_controller
{
	char *id;
	char *name;

	char *forMesh;

	int numSources;
	collada_source_array sources[10];

	int numVerts;
	collada_vertex_weight *vertWeights;

	bool hasBindShapeMatrix;
	float bindShapeMatrix[16];
};

enum collada_node_type
{
	COLLADA_NODE_NODE,
	COLLADA_NODE_JOINT,
};

enum collada_node_content_type
{
	COLLADA_INSTANCE_CONTROLLER,
	COLLADA_INSTANCE_LIGHT,
	COLLADA_INSTANCE_CAMERA,
	COLLADA_INSTANCE_GEOMETRY,
	COLLADA_GENERIC,
};

struct collada_instance_controller
{
	char *controllerURL;
	char *boneURL;
};

struct collada_instance_geometry
{
	char *meshURL;
};

struct collada_node
{
	char *id;
	char *name;
	collada_node_type type;
	v3 location;
	float rotationX;
	float rotationY;
	float rotationZ;
	v3 scale;

	int numNodes;
	collada_node *nodes;

	collada_node_content_type contentType;
	union
	{
		collada_instance_controller instanceController;
		collada_instance_geometry instanceGeometry;
	};
};

struct collada_visual_scene
{
	int numNodes;
	collada_node *nodes;
	char *id;
	char *name;
};

struct collada_file
{
	int numGeometries;
	collada_geometry *geometries;

	int numAnimations;
	collada_animation *animations;

	int numControllers;
	collada_controller *controllers;

	int numVisualScenes;
	collada_visual_scene *visualScenes;
};


collada_source_array *ColladaGetSourceArrayByParamName(const char *name, collada_source_array *sources, int numSources);
collada_source_array ColladaParseSourceArray(xmlnode *source, memory_arena *arena);
collada_poly_semantic ColladaParsePolySemantic(collada_geometry geometry, xmlnode *semantic);
collada_geometry ColladaParseGeometry(xmlnode *current, memory_arena *arena);
collada_controller ColladaParseController(xmlnode *controller, memory_arena *arena);
collada_animation ColladaParseAnimation(xmlnode *animation, memory_arena *arena);
collada_node ColladaParseNode(xmlnode *node, memory_arena *arena);
collada_visual_scene ColladaParseVisualScene(xmlnode *scene, memory_arena *arena);
collada_node *ColladaGetSceneObjectByName(collada_visual_scene *scene, const char *name);
collada_node *ColladaFindNodeByID(collada_node *node, const char *id);
collada_node *ColladaFindNodeByID(collada_visual_scene *scene, const char *id);

collada_animation *ColladaFindAnimationByTarget(collada_file *file, const char *target);
void ColladaPrintNode(collada_node *node, int indent);
collada_file ReadColladaFile(char *fileName, memory_arena *arena);
