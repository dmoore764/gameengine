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
#include "game_world.h"
#include "editor.h"

#include "assets.h"
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

int main (int argCount, char **args)
{
	uint32_t gameTime = 0;

	window w;
	winCreate(&w, V2I(720, 480));

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplSdlGL3_Init(w.win);
    ImGui::StyleColorsDark();

	memory_arena arena = {};
	InitArena(&arena, MEGABYTES(100));

	game_world *game = PUSH_ITEM(&arena, game_world);
	gwInit(game, &arena);

	int numObjects;
	obj3d_editor objects[1000];
	obj3d_editor *edit = NULL;

	{
		obj3d_editor *newobj = &objects[numObjects++];
		memset(newobj, '\0', sizeof(obj3d_editor));
		newobj->editing = true;
		newobj->rotation = CreateQuat(0, V3(1,0,0));
		newobj->scale = V3(1,1,1);
	}

	edit = &objects[0];

	framebuffer gameScreen;
	oglGenerateFrameBuffer(&gameScreen, MAG_FILTERING_NEAREST | MIN_FILTERING_NEAREST | CLAMP_TO_EDGE_S | CLAMP_TO_EDGE_T | TEX_2D, V2I(1024, 1024), true, true, STORAGE_RGB | STORAGE_FLOAT_16);

	framebuffer outlineDrawScreen;
	oglGenerateFrameBuffer(&outlineDrawScreen, MAG_FILTERING_NEAREST | MIN_FILTERING_NEAREST | CLAMP_TO_EDGE_S | CLAMP_TO_EDGE_T | TEX_2D, V2I(1024, 1024), true, false, STORAGE_R | STORAGE_UNSIGNED_BYTE);


	int indexData[6] = { 0, 2, 3, 0, 1, 2};

	basic_vertex vertDataFullScreen[4] = {
		{{-1,-1,0}, {0,0,0}, {0,0}, MAKE_COLOR(255,255,255,255)},
		{{-1,1,0},  {0,0,0}, {0,1}, MAKE_COLOR(255,255,255,255)},
		{{1,1,0},  {0,0,0}, {1,1}, MAKE_COLOR(255,255,255,255)},
		{{1,-1,0}, {0,0,0}, {1,0}, MAKE_COLOR(255,255,255,255)},
	};
	vertex_array_object vaoFullScreen = {};
	oglCreateVAOWithData(&vaoFullScreen, BASIC_VERTEX, 4, vertDataFullScreen, 6, indexData);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	bool running = true;
	while (running)
	{
		gameTime++;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
            ImGui_ImplSdlGL3_ProcessEvent(&event);
			winHandleEvent(&w, &event);
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		//glBindFramebuffer(GL_FRAMEBUFFER, gameScreen.glHandle);
		//glViewport(0,0,gameScreen.color.size.w, gameScreen.color.size.h);
		winClear(0,0,0);

		local_persistent float x = 0;
		local_persistent float y = -6;
		/*

		float newX = x * cosf(0.01f) - y * sinf(0.01f);
		float newY = y * cosf(0.01f) + x * sinf(0.01f);

		x = newX;
		y = newY;
		*/

		m4 la = MakeLookat(V3(x, y, 2), V3(0, 0, 1), V3(0, 0, 0));
		local_persistent float vfov = 50.0f;
		m4 ps = MakePerspective(vfov, (float)w.size.w/(float)w.size.h, 0.5f, 120.0f);
		m4 pv = ps*la;

		v2i mousePos;
		uint32_t mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);
		v2 ndcMousePos = winGetNormalizedScreenPoint(&w, mousePos);

		local_persistent bool mouseWasDown = false;
		bool mouseIsDown = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && !io.WantCaptureMouse;
		bool mousePressed = mouseIsDown && !mouseWasDown;
		bool mouseReleased = !mouseIsDown && mouseWasDown;
		mouseWasDown = mouseIsDown;

		local_persistent int editState = 0;
		local_persistent int localHandle = 0;

		//game->physWorld.dynamicsWorld->stepSimulation(1 / 60.f, 10);

		if (edit && edit->curVao)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, outlineDrawScreen.glHandle);
			glViewport(0,0,outlineDrawScreen.color.size.w, outlineDrawScreen.color.size.h);
			glClearColor(0,0,0,0);
			glDisable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT);

			glUseProgram(game->ass.basicMeshSolidColor.glHandle);
			GLint perspView = glGetUniformLocation(game->ass.basicMeshSolidColor.glHandle, "PerspView");
			GLint modelMat = glGetUniformLocation(game->ass.basicMeshSolidColor.glHandle, "ModelMat");
			GLint meshColor = glGetUniformLocation(game->ass.basicMeshSolidColor.glHandle, "MeshColor");
			glUniformMatrix4fv(perspView, 1, false, &pv[0]);
			glUniformMatrix4fv(modelMat, 1, false, &edit->modelMat[0]);
			v4 col = V4(1,1,1,1);
			glUniform4fv(meshColor, 1, (float *)&col.x);
			rendRenderVAO(edit->curVao);
			glEnable(GL_DEPTH_TEST);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0,0, w.backingRes.w, w.backingRes.h);
		}

		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Draw all the objects
		for (int i = 0; i < numObjects; i++)
		{
			obj3d_editor *obj = &objects[i];
			obj->modelMat = RotationPositionScaleToM4(obj->rotation, obj->position, obj->scale);
			obj->curVao = NULL;
			switch ((object3d_types)obj->objType)
			{
				case object3d_types::NON_VISUAL:
				{
				} break;

				case object3d_types::BONE_MODEL:
				{
					if (obj->numLODs > 0)
					{
						bone_model *bm = (bone_model *)GetFromHash(&game->ass.boneModelHash, obj->lods[0].name);
						if (bm)
							obj->curVao = &bm->mesh;
					}
				} break;

				case object3d_types::BASIC_GEOMETRY:
				{
					if (obj->numLODs > 0)
					{
						basic_mesh *bm = (basic_mesh *)GetFromHash(&game->ass.basicMeshHash, obj->lods[0].name);
						if (bm)
							obj->curVao = &bm->mesh;
					}
				} break;
			}

			if (obj->curVao)
			{
				glUseProgram(game->ass.basicMeshColoredVerts.glHandle);
				GLint perspView = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "PerspView");
				GLint modelMat = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "ModelMat");
				glUniformMatrix4fv(perspView, 1, false, &pv[0]);
				glUniformMatrix4fv(modelMat, 1, false, &obj->modelMat[0]);
				rendRenderVAO(obj->curVao);
			}
		}

		//Draw the physics shape
		if (edit)
		{
			if (edit->physType == (int)object3d_physics::DYNAMIC || edit->physType == (int)object3d_physics::KINETIC)
			{
				m4 model = RotationPositionToM4(edit->rotation, edit->position);

				glUseProgram(game->ass.basicMeshColoredVerts.glHandle);
				GLint perspView = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "PerspView");
				GLint modelMat = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "ModelMat");
				glUniformMatrix4fv(perspView, 1, false, &pv[0]);
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

				switch ((physics_shapes)edit->shape)
				{
					case SHAPE_PLANE_X:
					{
						m4 rotate = QuatToM4(CreateQuat(90, V3(0,1,0)));
						model = model * MakeTranslation(edit->offset.x, 0, 0);
						model = model * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapePlane.mesh);
					} break;
					case SHAPE_PLANE_Y:
					{
						m4 rotate = QuatToM4(CreateQuat(90, V3(-1,0,0)));
						model = model * MakeTranslation(0, edit->offset.x, 0);
						model = model * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapePlane.mesh);
					} break;
					case SHAPE_PLANE_Z:
					{
						model = model * MakeTranslation(0, 0, edit->offset.x);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapePlane.mesh);
					} break;
					case SHAPE_CAPSULE_X:
					{
						m4 rotate = QuatToM4(CreateQuat(90, V3(0,1,0)));
						m4 modelBottom = model * MakeTranslation(edit->offset.x - edit->dimensions.y * 0.5f, edit->offset.y, edit->offset.z);
						modelBottom = modelBottom * MakeScale(edit->dimensions.x, edit->dimensions.x, edit->dimensions.x);
						modelBottom = modelBottom * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelBottom[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleBottom.mesh);

						m4 modelMiddle = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						modelMiddle = modelMiddle * MakeScale(edit->dimensions.y, edit->dimensions.x, edit->dimensions.x);
						modelMiddle = modelMiddle * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelMiddle[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleMiddle.mesh);

						m4 modelTop = model * MakeTranslation(edit->offset.x + edit->dimensions.y * 0.5f, edit->offset.y, edit->offset.z);
						modelTop = modelTop * MakeScale(edit->dimensions.x, edit->dimensions.x, edit->dimensions.x);
						modelTop = modelTop * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelTop[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleTop.mesh);
					} break;
					case SHAPE_CAPSULE_Y:
					{
						m4 rotate = QuatToM4(CreateQuat(90, V3(-1,0,0)));
						m4 modelBottom = model * MakeTranslation(edit->offset.x, edit->offset.y - edit->dimensions.y * 0.5f, edit->offset.z);
						modelBottom = modelBottom * MakeScale(edit->dimensions.x, edit->dimensions.x, edit->dimensions.x);
						modelBottom = modelBottom * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelBottom[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleBottom.mesh);

						m4 modelMiddle = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						modelMiddle = modelMiddle * MakeScale(edit->dimensions.x, edit->dimensions.y, edit->dimensions.x);
						modelMiddle = modelMiddle * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelMiddle[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleMiddle.mesh);

						m4 modelTop = model * MakeTranslation(edit->offset.x, edit->offset.y + edit->dimensions.y * 0.5f, edit->offset.z);
						modelTop = modelTop * MakeScale(edit->dimensions.x, edit->dimensions.x, edit->dimensions.x);
						modelTop = modelTop * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelTop[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleTop.mesh);
					} break;
					case SHAPE_CAPSULE_Z:
					{
						m4 modelBottom = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z - edit->dimensions.y * 0.5f);
						modelBottom = modelBottom * MakeScale(edit->dimensions.x, edit->dimensions.x, edit->dimensions.x);
						glUniformMatrix4fv(modelMat, 1, false, &modelBottom[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleBottom.mesh);

						m4 modelMiddle = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						modelMiddle = modelMiddle * MakeScale(edit->dimensions.x, edit->dimensions.x, edit->dimensions.y);
						glUniformMatrix4fv(modelMat, 1, false, &modelMiddle[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleMiddle.mesh);

						m4 modelTop = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z + edit->dimensions.y * 0.5f);
						modelTop = modelTop * MakeScale(edit->dimensions.x, edit->dimensions.x, edit->dimensions.x);
						glUniformMatrix4fv(modelMat, 1, false, &modelTop[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleTop.mesh);
					} break;
					case SHAPE_CONE_X:
					{
						model = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						model = model * MakeScale(edit->dimensions.y, edit->dimensions.x, edit->dimensions.x);
						model = model * QuatToM4(CreateQuat(90, V3(0,1,0)));
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCone.mesh);
					} break;
					case SHAPE_CONE_Y:
					{
						model = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						model = model * MakeScale(edit->dimensions.x, edit->dimensions.y, edit->dimensions.x);
						model = model * QuatToM4(CreateQuat(90, V3(-1,0,0)));
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCone.mesh);
					} break;
					case SHAPE_CONE_Z:
					{
						model = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						model = model * MakeScale(edit->dimensions.x, edit->dimensions.x, edit->dimensions.y);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCone.mesh);
					} break;
					case SHAPE_SPHERE:
					{
						model = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						model = model * MakeScale(edit->dimensions.x, edit->dimensions.x, edit->dimensions.x);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeSphere.mesh);
					} break;
					case SHAPE_CYLINDER_X:
					{
						model = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						model = model * MakeScale(edit->dimensions.y, edit->dimensions.x, edit->dimensions.x);
						model = model * QuatToM4(CreateQuat(90, V3(0,1,0)));
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCylinder.mesh);
					} break;
					case SHAPE_CYLINDER_Y:
					{
						model = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						model = model * MakeScale(edit->dimensions.x, edit->dimensions.y, edit->dimensions.x);
						model = model * QuatToM4(CreateQuat(90, V3(1,0,0)));
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCylinder.mesh);
					} break;
					case SHAPE_CYLINDER_Z:
					{
						model = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						model = model * MakeScale(edit->dimensions.x, edit->dimensions.x, edit->dimensions.y);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCylinder.mesh);
					} break;
					case SHAPE_BOX:
					{
						model = model * MakeTranslation(edit->offset.x, edit->offset.y, edit->offset.z);
						model = model * MakeScale(edit->dimensions.x, edit->dimensions.y, edit->dimensions.z);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeRectangularPrism.mesh);
					} break;
				}
				glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			}

			if (editState == 0)
			{
				local_persistent manipulator objHandle;
				switch (objHandle.state)
				{
					case manipulator_states::GRABBABLE:
					{
						v3 xAxisDir = V3(2,0,0);
						v3 yAxisDir = V3(0,2,0);
						v3 zAxisDir = V3(0,0,2);
						if (localHandle)
						{
							xAxisDir = RotateByQuat(xAxisDir, edit->rotation);
							yAxisDir = RotateByQuat(yAxisDir, edit->rotation);
							zAxisDir = RotateByQuat(zAxisDir, edit->rotation);
						}
						objHandle.xAxisHandle = {WorldPointToNDCScreenPoint(edit->position, pv), WorldPointToNDCScreenPoint(edit->position + xAxisDir, pv)};
						objHandle.yAxisHandle = {WorldPointToNDCScreenPoint(edit->position, pv), WorldPointToNDCScreenPoint(edit->position + yAxisDir, pv)};
						objHandle.zAxisHandle = {WorldPointToNDCScreenPoint(edit->position, pv), WorldPointToNDCScreenPoint(edit->position + zAxisDir, pv)};

						float distToXHandle = DistanceToLineSeg2D(objHandle.xAxisHandle, ndcMousePos, NULL);
						float distToYHandle = DistanceToLineSeg2D(objHandle.yAxisHandle, ndcMousePos, NULL);
						float distToZHandle = DistanceToLineSeg2D(objHandle.zAxisHandle, ndcMousePos, NULL);

						bool canManipulate = false;
						if (distToXHandle <= distToYHandle && distToXHandle <= distToZHandle)
						{
							if (distToXHandle < 0.05f)
							{
								canManipulate = true;
								objHandle.whichAxis = 0;
								objHandle.axisLine = {edit->position, edit->position + xAxisDir};
							}
						}
						if (distToYHandle <= distToXHandle && distToYHandle <= distToZHandle)
						{
							if (distToYHandle < 0.05f)
							{
								canManipulate = true;
								objHandle.whichAxis = 1;
								objHandle.axisLine = {edit->position, edit->position + yAxisDir};
							}
						}
						if (distToZHandle <= distToXHandle && distToZHandle <= distToYHandle)
						{
							if (distToZHandle < 0.05f)
							{
								canManipulate = true;
								objHandle.whichAxis = 2;
								objHandle.axisLine = {edit->position, edit->position + zAxisDir};
							}
						}

						if (canManipulate && mousePressed)
						{
							objHandle.state = manipulator_states::IS_MANIPULATING;

							v2 pointOnAxis;
							DistanceToLine(objHandle.axisHandle[objHandle.whichAxis], ndcMousePos, &pointOnAxis);
							line_seg_3d mouseLine = WindowPointToWorldLineSeg(pointOnAxis, pv);

							bool linesParallel;
							v3 closestPtA, closestPtB;
							ClosestPointsOn3DLines(objHandle.axisLine, mouseLine, &closestPtA, &closestPtB, &linesParallel);

							if (!linesParallel)
								objHandle.holdingOffset = closestPtA - edit->position;
							else
								objHandle.state = manipulator_states::GRABBABLE;
						}

					} break;

					case manipulator_states::IS_MANIPULATING:
					{
						v2 pointOnAxis;
						DistanceToLine(objHandle.axisHandle[objHandle.whichAxis], ndcMousePos, &pointOnAxis);
						line_seg_3d mouseLine = WindowPointToWorldLineSeg(pointOnAxis, pv);

						bool linesParallel;
						v3 closestPtA, closestPtB;
						ClosestPointsOn3DLines(objHandle.axisLine, mouseLine, &closestPtA, &closestPtB, &linesParallel);

						if (!linesParallel)
							edit->position = closestPtA - objHandle.holdingOffset;

						if (mouseReleased)
							objHandle.state = manipulator_states::GRABBABLE;
					} break;
				}

				{
					m4 model;
					if (localHandle)
						model = QuatToM4(edit->rotation);
					else
						model = M4();
					model.col[3] = V4(edit->position.x, edit->position.y, edit->position.z, 1.0f);

					glUseProgram(game->ass.basicMeshColoredVerts.glHandle);
					GLint perspView = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "PerspView");
					GLint modelMat = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "ModelMat");
					glUniformMatrix4fv(perspView, 1, false, &pv[0]);
					glUniformMatrix4fv(modelMat, 1, false, &model[0]);

					rendRenderVAO(&game->ass.manipulatorX.mesh);
					rendRenderVAO(&game->ass.manipulatorY.mesh);
					rendRenderVAO(&game->ass.manipulatorZ.mesh);

					glUseProgram(game->ass.hiddenMeshColoredVerts.glHandle);
					perspView = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "PerspView");
					modelMat = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "ModelMat");
					glUniformMatrix4fv(perspView, 1, false, &pv[0]);
					glUniformMatrix4fv(modelMat, 1, false, &model[0]);
					glDepthFunc(GL_GREATER);
					rendRenderVAO(&game->ass.manipulatorX.mesh);
					rendRenderVAO(&game->ass.manipulatorY.mesh);
					rendRenderVAO(&game->ass.manipulatorZ.mesh);
					glDepthFunc(GL_LESS);
				}
			}
			else if (editState == 1)
			{
				local_persistent rotator objRotator;

				switch (objRotator.state)
				{
					case rotator_states::PICK_AXIS:
					{
						m4 model;
						if (localHandle)
							model = QuatToM4(edit->rotation);
						else
							model = M4();
						model.col[3] = V4(edit->position.x, edit->position.y, edit->position.z, 1.0f);

						glUseProgram(game->ass.basicMeshColoredVerts.glHandle);
						GLint perspView = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "PerspView");
						GLint modelMat = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "ModelMat");
						glUniformMatrix4fv(perspView, 1, false, &pv[0]);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);

						rendRenderVAO(&game->ass.manipulatorX.mesh);
						rendRenderVAO(&game->ass.manipulatorY.mesh);
						rendRenderVAO(&game->ass.manipulatorZ.mesh);

						glUseProgram(game->ass.hiddenMeshColoredVerts.glHandle);
						perspView = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "PerspView");
						modelMat = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "ModelMat");
						glUniformMatrix4fv(perspView, 1, false, &pv[0]);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						glDepthFunc(GL_GREATER);
						rendRenderVAO(&game->ass.manipulatorX.mesh);
						rendRenderVAO(&game->ass.manipulatorY.mesh);
						rendRenderVAO(&game->ass.manipulatorZ.mesh);
						glDepthFunc(GL_LESS);

						const Uint8 *state = SDL_GetKeyboardState(NULL);
						if (state[SDL_SCANCODE_X])
						{
							objRotator.whichAxis = 0;
							objRotator.state = rotator_states::AXIS_PICKED;
						}
						else if (state[SDL_SCANCODE_Y])
						{
							objRotator.whichAxis = 1;
							objRotator.state = rotator_states::AXIS_PICKED;
						}
						else if (state[SDL_SCANCODE_Z])
						{
							objRotator.whichAxis = 2;
							objRotator.state = rotator_states::AXIS_PICKED;
						}
					} break;

					case rotator_states::AXIS_PICKED:
					{
						vertex_array_object *axisToDraw;
						vertex_array_object *axisPlaneToDraw;
						if (objRotator.whichAxis == 0)
						{
							axisToDraw = &game->ass.manipulatorX.mesh;
							axisPlaneToDraw = &game->ass.xaxis.mesh;
						}
						else if (objRotator.whichAxis == 1)
						{
							axisToDraw = &game->ass.manipulatorY.mesh;
							axisPlaneToDraw = &game->ass.yaxis.mesh;
						}
						else if (objRotator.whichAxis == 2)
						{
							axisToDraw = &game->ass.manipulatorZ.mesh;
							axisPlaneToDraw = &game->ass.zaxis.mesh;
						}

						glUseProgram(game->ass.basicMeshColoredVerts.glHandle);
						GLuint perspView = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "PerspView");
						GLuint modelMat = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "ModelMat");
						glUniformMatrix4fv(perspView, 1, false, &pv[0]);
						m4 model;
						if (localHandle)
							model = QuatToM4(edit->rotation);
						else
							model = M4();
						model.col[3] = V4(edit->position.x, edit->position.y, edit->position.z,1);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(axisPlaneToDraw);
						rendRenderVAO(axisToDraw);

						glUseProgram(game->ass.hiddenMeshColoredVerts.glHandle);
						perspView = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "PerspView");
						modelMat = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "ModelMat");
						glUniformMatrix4fv(perspView, 1, false, &pv[0]);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						glDepthFunc(GL_GREATER);
						rendRenderVAO(axisToDraw);
						glDepthFunc(GL_LESS);

						if (mousePressed)
						{
							objRotator.axisToDraw = axisToDraw;
							objRotator.axisPlaneToDraw = axisPlaneToDraw;

							v3 normal;
							v3 up;
							v3 right;
							if (objRotator.whichAxis == 0) //x
							{
								normal = V3(1,0,0);
								up = V3(0,1,0);
								right = V3(0,0,-1);
							}
							if (objRotator.whichAxis == 1) //y
							{
								normal = V3(0,1,0);
								up = V3(0,0,1);
								right = V3(-1,0,0);
							}
							if (objRotator.whichAxis == 2) //z
							{
								normal = V3(0,0,1);
								up = V3(1,0,0);
								right = V3(0,-1,0);
							}

							if (localHandle)
							{
								normal = RotateByQuat(normal, edit->rotation);
								up = RotateByQuat(up, edit->rotation);
								right = RotateByQuat(right, edit->rotation);
							}

							plane rotatePlane = PlaneFromNormalAndPoint(normal, edit->position);

							objRotator.initialUp = up;
							objRotator.initialRight = right;
							objRotator.initialNormal = normal;
							objRotator.rotatePlane = rotatePlane;

							line_seg_3d mouseLine = WindowPointToWorldLineSeg(ndcMousePos, pv);
							bool parallel;
							v3 intersection;
							LinePlaneIntersection(mouseLine, rotatePlane, &intersection, &parallel);

							if (!parallel)
							{
								v3 dragVector = intersection - edit->position;
								objRotator.initialRotation = edit->rotation;
								v2 rotationPt = {Inner(right, dragVector), Inner(up, dragVector)};
								objRotator.rotationOffset = atan2f(rotationPt.y, rotationPt.x);
								objRotator.state = rotator_states::IS_ROTATING;
							}
						}

						const Uint8 *state = SDL_GetKeyboardState(NULL);
						if (state[SDL_SCANCODE_X])
							objRotator.whichAxis = 0;
						else if (state[SDL_SCANCODE_Y])
							objRotator.whichAxis = 1;
						else if (state[SDL_SCANCODE_Z])
							objRotator.whichAxis = 2;
					} break;

					case rotator_states::IS_ROTATING:
					{
						line_seg_3d mouseLine = WindowPointToWorldLineSeg(ndcMousePos, pv);
						bool parallel;
						v3 intersection;
						LinePlaneIntersection(mouseLine, objRotator.rotatePlane, &intersection, &parallel);

						if (!parallel)
						{
							v3 dragVector = intersection - edit->position;
							v2 rotationPt = {Inner(objRotator.initialRight, dragVector), Inner(objRotator.initialUp, dragVector)};
							float rotation = atan2f(rotationPt.y, rotationPt.x);
							quat newRotation = CreateQuatRAD(rotation - objRotator.rotationOffset, objRotator.initialNormal);
							edit->rotation = RotateQuatByQuat(objRotator.initialRotation, newRotation);
						}

						glUseProgram(game->ass.basicMeshColoredVerts.glHandle);
						GLuint perspView = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "PerspView");
						GLuint modelMat = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "ModelMat");
						glUniformMatrix4fv(perspView, 1, false, &pv[0]);
						m4 model;
						if (localHandle)
							model = QuatToM4(edit->rotation);
						else
							model = M4();
						model.col[3] = V4(edit->position.x, edit->position.y, edit->position.z,1);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(objRotator.axisPlaneToDraw);
						rendRenderVAO(objRotator.axisToDraw);

						glUseProgram(game->ass.hiddenMeshColoredVerts.glHandle);
						perspView = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "PerspView");
						modelMat = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "ModelMat");
						glUniformMatrix4fv(perspView, 1, false, &pv[0]);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						glDepthFunc(GL_GREATER);
						rendRenderVAO(objRotator.axisToDraw);
						glDepthFunc(GL_LESS);

						if (mouseReleased)
							objRotator.state = rotator_states::AXIS_PICKED;
					} break;
				}
			}
			else if (editState == 2) //scaling
			{
				m4 model;
				model = RotationPositionToM4(edit->rotation, edit->position);

				glUseProgram(game->ass.basicMeshColoredVerts.glHandle);
				GLint perspView = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "PerspView");
				GLint modelMat = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "ModelMat");
				glUniformMatrix4fv(perspView, 1, false, &pv[0]);
				glUniformMatrix4fv(modelMat, 1, false, &model[0]);

				rendRenderVAO(&game->ass.manipulatorX.mesh);
				rendRenderVAO(&game->ass.manipulatorY.mesh);
				rendRenderVAO(&game->ass.manipulatorZ.mesh);

				glUseProgram(game->ass.hiddenMeshColoredVerts.glHandle);
				perspView = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "PerspView");
				modelMat = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "ModelMat");
				glUniformMatrix4fv(perspView, 1, false, &pv[0]);
				glUniformMatrix4fv(modelMat, 1, false, &model[0]);
				glDepthFunc(GL_GREATER);
				rendRenderVAO(&game->ass.manipulatorX.mesh);
				rendRenderVAO(&game->ass.manipulatorY.mesh);
				rendRenderVAO(&game->ass.manipulatorZ.mesh);
				glDepthFunc(GL_LESS);
			}
		}

		/*
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0, w.backingRes.w, w.backingRes.h);
		winClear(0,0,0);

		{
			glUseProgram(texturedTriangles.glHandle);
			GLint perspView = glGetUniformLocation(texturedTriangles.glHandle, "PerspView");
			m4 identity = M4();
			glUniformMatrix4fv(perspView, 1, false, &identity[0]);
			rendSetTexture(&gameScreen.color, gameTime);

			rendRenderVAO(&vaoFullScreen);
		}
		*/

		{
			glUseProgram(game->ass.outlineDrawer.glHandle);
			GLint perspView = glGetUniformLocation(game->ass.outlineDrawer.glHandle, "PerspView");
			GLint resolution = glGetUniformLocation(game->ass.outlineDrawer.glHandle, "Resolution");
			m4 identity = M4();
			glUniformMatrix4fv(perspView, 1, false, &identity[0]);
			v2 res = V2(outlineDrawScreen.color.size.w, outlineDrawScreen.color.size.h);
			glUniform2fv(resolution, 1, (float *)&res.x);
			rendSetTexture(&outlineDrawScreen.color, gameTime);

			rendRenderVAO(&vaoFullScreen);
		}

		local_persistent bool showDebug = true;
		local_persistent bool tabWasDown = false;
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		bool tabIsDown = state[SDL_SCANCODE_TAB];
		bool tabPressed = tabIsDown & !tabWasDown;
		tabWasDown = tabIsDown;

		if (tabPressed)
			showDebug = !showDebug;

		if (showDebug)
        {
			ImGui_ImplSdlGL3_NewFrame(w.win);

			static bool show_demo_window = true;
			ImGui::ShowDemoWindow(&show_demo_window);

            static int counter = 0;
            ImGui::SliderFloat("v-FOV", &vfov, 30.0f, 100.0f);
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::RadioButton("global", &localHandle, 0); ImGui::SameLine();
            ImGui::RadioButton("local", &localHandle, 1);

            ImGui::RadioButton("translate", &editState, 0); ImGui::SameLine();
            ImGui::RadioButton("rotate", &editState, 1); ImGui::SameLine();
            ImGui::RadioButton("scale", &editState, 2);

			if (editState == 2)
				ImGui::DragFloat3("scale", (float *)&edit->scale.x, 0.01f);

			if (edit)
			{
				ImGui::Checkbox("Starts Active", &edit->startsActive);
				ImGui::Checkbox("Starts Visible", &edit->startsVisible);
				ImGui::RadioButton("non-visual", &edit->objType, 0); ImGui::SameLine();
				ImGui::RadioButton("bone", &edit->objType, 1); ImGui::SameLine();
				ImGui::RadioButton("mesh", &edit->objType, 2);
				ImGui::RadioButton("non-physical", &edit->physType, 0); ImGui::SameLine();
				ImGui::RadioButton("dynamic", &edit->physType, 1); ImGui::SameLine();
				ImGui::RadioButton("kinetic", &edit->physType, 2);

				if (edit->physType == (int)object3d_physics::DYNAMIC || edit->physType == (int)object3d_physics::KINETIC)
				{
					ImGui::Combo("phys shape", &edit->shape, "PLANE_X\0PLANE_Y\0PLANE_Z\0CAPSULE_X\0CAPSULE_Y\0CAPSULE_Z\0CONE_X\0CONE_Y\0CONE_Z\0SPHERE\0CYLINDER_X\0CYLINDER_Y\0CYLINDER_Z\0BOX\0\0");
					switch ((physics_shapes)edit->shape)
					{
						case SHAPE_PLANE_X:
						case SHAPE_PLANE_Y:
						case SHAPE_PLANE_Z:
						{
							ImGui::DragFloat("offset", (float *)&edit->offset.x, 0.01f);
						} break;
						case SHAPE_CAPSULE_X:
						case SHAPE_CAPSULE_Y:
						case SHAPE_CAPSULE_Z:
						case SHAPE_CONE_X:
						case SHAPE_CONE_Y:
						case SHAPE_CONE_Z:
						case SHAPE_CYLINDER_X:
						case SHAPE_CYLINDER_Y:
						case SHAPE_CYLINDER_Z:
						{
							ImGui::DragFloat("radius", (float *)&edit->dimensions.x, 0.01f);
							ImGui::DragFloat("height", (float *)&edit->dimensions.y, 0.01f);
							ImGui::DragFloat3("offset", (float *)&edit->offset.x, 0.01f);
							ImGui::DragFloat("mass", &edit->mass, 0.01f, 0.0f, 500.0f);
						} break;
						case SHAPE_SPHERE:
						{
							ImGui::DragFloat("radius", (float *)&edit->dimensions.x, 0.01f);
							ImGui::DragFloat3("offset", (float *)&edit->offset.x, 0.01f);
							ImGui::DragFloat("mass", &edit->mass, 0.01f, 0.0f, 500.0f);
						} break;
						case SHAPE_BOX:
						{
							ImGui::DragFloat3("dimensions", (float *)&edit->dimensions.x, 0.01f);
							ImGui::DragFloat3("offset", (float *)&edit->offset.x, 0.01f);
							ImGui::DragFloat("mass", &edit->mass, 0.01f, 0.0f, 500.0f);
						} break;
					}
					ImGui::DragFloat("restitution", &edit->restitution, 0.01f, 0.0f, 1.0f);
				}

				if (edit->numLODs < ArrayCount(edit->lods))
				{
					if (ImGui::Button("Add LOD mesh"))
						edit->numLODs++;
				}

				for (int i = 0; i < edit->numLODs; i++)
				{
					obj3d_mesh_assignment *lodMesh = &edit->lods[i];
					ImGui::DragFloat("Distance", &lodMesh->validDist, 0.05f, 0.0f);
					ImGui::InputText("Mesh/Bone", lodMesh->name, IM_ARRAYSIZE(lodMesh->name));
				}
			}

			ImGui::Render();
			ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
        }

		winSwapBuffers(&w);
	}


    ImGui_ImplSdlGL3_Shutdown();
    ImGui::DestroyContext();

	gwClose(game);

	winClose(&w);
	return 0;
}
