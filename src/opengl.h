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

enum texture_flags
{
	MAG_FILTERING_NEAREST			= 0x1,
	MAG_FILTERING_LINEAR			= 0x2,
	//MAG_FILTERING_ANISOTROPIC		= 0x4,
	MIN_FILTERING_NEAREST			= 0x8,
	MIN_FILTERING_LINEAR			= 0x10,
	MIN_FILTERING_MIPMAP_NEAREST	= 0x20,
	MIN_FILTERING_MIPMAP_LINEAR		= 0x40,
	MIN_FILTERING_2_MIPMAPS_NEAREST	= 0x80,
	MIN_FILTERING_2_MIPMAPS_LINEAR	= 0x100,
	CLAMP_TO_EDGE_S					= 0x200,
	CLAMP_TO_EDGE_T					= 0x400,
	CLAMP_TO_EDGE_R					= 0x800,
	REPEAT_S						= 0x1000,
	REPEAT_T						= 0x2000,
	REPEAT_R						= 0x4000,
	MIRRORED_REPEAT_S				= 0x8000,
	MIRRORED_REPEAT_T				= 0x10000,
	MIRRORED_REPEAT_R				= 0x20000,
	TEX_1D							= 0x40000,
	TEX_1D_ARRAY					= 0x80000,
	TEX_2D							= 0x100000,
	TEX_2D_ARRAY					= 0x200000,
	TEX_CUBEMAP						= 0x400000,
	TEX_CUBEMAP_ARRAY				= 0x800000,
	TEX_2D_MULTISAMPLE				= 0x1000000,
	TEX_2D_MULTISAMPLE_ARRAY		= 0x2000000,
	TEX_3D							= 0x4000000,
};

enum texture_load_flags
{
	TEX_LOADED						= 0x1,
	TEX_FROM_FILE					= 0x2,
	TEX_SHOULD_GEN_MIPMAPS			= 0x4,
};

struct texture
{
	char *fileName;
	GLuint glHandle;
	uint32_t loadFlags;
	uint32_t flags;
	uint32_t lastUsed;
	v2i size;
	int n;
};

void oglLoadTexture2DIntoGPU(texture *t);
void oglGenerateMipmaps(texture *t);

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

enum vao_render_type
{
	VAO_DRAW_ARRAYS,
	VAO_DRAW_ELEMENTS,
};

struct vertex_array_object
{
	vao_render_type type;
	GLuint vaoHandle;
	GLuint vbHandle;
	GLuint ebHandle;
	int vertexAttributes;
	int numVertices;
	int numIndices;
};

void oglCreateVAOWithData(vertex_array_object *vao, vertex_type type, int numVertices, void *data);
void oglCreateVAOWithData(vertex_array_object *vao, vertex_type type, int numVertices, void *data, int numIndices, void *indices);
