#pragma once

#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#include "game_math.h"

enum vertex_type
{
	BASIC_VERTEX,
	SKELETAL_VERTEX,
};

struct basic_vertex
{
	v3 position;
	v3 normal;
	v2 uv;
	uint32_t color;
};

struct skeletal_vertex
{
	v3 position;
	v3 normal;
	v2 uv;
	uint32_t color;
	float weights[3];
	int jointIndices[3];
};

struct texture
{
	int glHandle;
	v2 size;
};

struct framebuffer
{
	int glHandle;
	texture *t;
};

enum vertex_attributes
{
	ATTR_POSITION = 0x1,
	ATTR_NORMAL = 0x2,
	ATTR_UV = 0x4,
	ATTR_COLOR = 0x8,
	ATTR_JOINT_WEIGHTS = 0x10,
	ATTR_JOINT_INDICES = 0x20,
};

#define LAYOUT_LOC_POSITION 0
#define LAYOUT_LOC_NORMAL 1
#define LAYOUT_LOC_UV 2
#define LAYOUT_LOC_COLOR 3
#define LAYOUT_LOC_JOINT_WEIGHTS 4
#define LAYOUT_LOC_JOINT_INDICES 5

struct shader
{
	int glHandle;
	int vertexAttributes;
};

void oglLoadShader(shader *s, char *vertShaderFile, char *fragShaderFile);

struct vertex_array_object
{
	GLuint vaoHandle;
	GLuint vbHandle;
	GLuint ebHandle;
	int vertexAttributes;
	int numVertices;
};

void oglCreateVAOWithData(vertex_array_object *vao, vertex_type type, int numVertices, void *data);
