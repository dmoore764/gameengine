#include "stdio.h"
#include "window.h"
#include "opengl.h"
#include "compiler_helper.h"
#include "game_math.h"
#include "memory_arena.h"
#include "collada.h"
#include "mesh.h"
#include "object3d.h"
#include "render.h"

int main (int argCount, char **args)
{
	uint32_t gameTime = 0;

	window w;
	WinCreate(&w, V2I(640, 480));

	memory_arena arena = {};
	InitArena(&arena, MEGABYTES(100));

	collada_file bonetest = ReadColladaFile("../assets/meshes/bonetest.dae", &arena);
	bone_model wormModel = BoneModelGetFromColladaByName(&bonetest, "Worm", &arena);
	bone_model_instance wormInst = BoneModelInstanceNew(&wormModel);
	object3d worm = {};
	worm.type = OBJECT3D_BONE_MODEL;
	worm.position = V3(0,0,0);
	worm.rotation = CreateQuat(0, V3(0,0,1));
	worm.boneModelInstance = wormInst;

	texture uvTex = {};
	uvTex.fileName = "../assets/textures/UVMap.png";
	uvTex.flags = MAG_FILTERING_NEAREST | MIN_FILTERING_MIPMAP_LINEAR | CLAMP_TO_EDGE_S | CLAMP_TO_EDGE_T | TEX_2D;
	uvTex.loadFlags = TEX_FROM_FILE | TEX_SHOULD_GEN_MIPMAPS;

	shader s = {};
	oglLoadShader(&s, "../shaders/solid_color_vert.glsl", "../shaders/solid_color_frag.glsl");
	s.vertexAttributes = ATTR_POSITION | ATTR_COLOR;

	shader s1 = {};
	oglLoadShader(&s1, "../shaders/bone_model_vert.glsl", "../shaders/bone_model_frag.glsl");
	s1.vertexAttributes = ATTR_POSITION | ATTR_NORMAL | ATTR_UV | ATTR_COLOR | ATTR_JOINT_WEIGHTS | ATTR_JOINT_INDICES;

	basic_vertex vertData[4] = {
		{{-1,2,0}, {0,0,0}, {0,0}, MAKE_COLOR(255,255,255,255)},
		{{1,2,0}, {0,0,0}, {0,0}, MAKE_COLOR(0,255,255,255)},
		{{1,2,2}, {0,0,0}, {0,0}, MAKE_COLOR(255,0,255,255)},
		{{-1,2,2}, {0,0,0}, {0,0}, MAKE_COLOR(255,0,255,255)},
	};

	uint32_t indexData[6] = {
		0, 1, 3, 1, 2, 3
	};

	vertex_array_object vao = {};

	oglCreateVAOWithData(&vao, BASIC_VERTEX, 4, vertData, 6, indexData);

	glEnable(GL_DEPTH_TEST);

	bool running = true;
	while (running)
	{
		gameTime++;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		WinClear(0,0,0);

		local_persistent float x = 0;
		local_persistent float y = -6;

		float newX = x * cosf(0.01f) - y * sinf(0.01f);
		float newY = y * cosf(0.01f) + x * sinf(0.01f);

		x = newX;
		y = newY;

		m4 la = MakeLookat(V3(x, y, 2), V3(0, 0, 1), V3(0, 0, 0));
		m4 ps = MakePerspective(50.0f, (float)w.size.w/(float)w.size.h, 0.5f, 120.0f);

		m4 pv = ps*la;

		{
			glUseProgram(s.glHandle);
			GLint perspView = glGetUniformLocation(s.glHandle, "PerspView");
			glUniformMatrix4fv(perspView, 1, false, &pv[0]);

			rendRenderVAO(&vao);
		}

		{
			Object3DUpdate(&worm, 0.016f);
			Object3DCalculateModelMat(&worm);
			glUseProgram(s1.glHandle);
			GLint modelMat = glGetUniformLocation(s1.glHandle, "ModelMat");
			GLint jointTransforms = glGetUniformLocation(s1.glHandle, "JointTransforms");
			GLint perspView = glGetUniformLocation(s1.glHandle, "PerspView");
			GLint lightDir = glGetUniformLocation(s1.glHandle, "LightDir");
			rendSetTexture(&uvTex, gameTime);

			glUniformMatrix4fv(modelMat, 1, false, &worm.modelMat[0]);
			glUniformMatrix4fv(jointTransforms, worm.boneModelInstance.model->numJoints, false, &worm.boneModelInstance.boneTransforms[0][0]);
			glUniformMatrix4fv(perspView, 1, false, &pv[0]);
			v3 lightDirection = Normalize(V3(2,3,-1));
			glUniform3fv(lightDir, 1, &lightDirection.x);

			rendRenderVAO(&worm.boneModelInstance.model->mesh);
		}

		WinSwapBuffers(&w);

	}

	WinClose(&w);
	return 0;
}
