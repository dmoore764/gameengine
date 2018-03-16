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
	w.scrollX.scroll_sensitivity = 100;
	w.scrollY.scroll_sensitivity = 30;
	w.scrollX.scroll_friction = 0.005f;
	w.scrollY.scroll_friction = 0.005f;

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplSdlGL3_Init(w.win);
    ImGui::StyleColorsDark();

	memory_arena arena = {};
	InitArena(&arena, MEGABYTES(100));

	game_world *game = PUSH_ITEM(&arena, game_world);
	gwInit(game, &arena);

	editor e;
	editorInit(&e);

	framebuffer gameScreen;
	oglGenerateFrameBuffer(&gameScreen, MAG_FILTERING_NEAREST | MIN_FILTERING_NEAREST | CLAMP_TO_EDGE_S | CLAMP_TO_EDGE_T | TEX_2D, V2I(1024, 1024), true, true, STORAGE_RGB | STORAGE_FLOAT_16);

	framebuffer outlineDrawScreen;
	oglGenerateFrameBuffer(&outlineDrawScreen, MAG_FILTERING_NEAREST | MIN_FILTERING_NEAREST | CLAMP_TO_EDGE_S | CLAMP_TO_EDGE_T | TEX_2D, V2I(1024, 1024), true, false, STORAGE_RG | STORAGE_UNSIGNED_BYTE);


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

		v2 mouseWheel = {0,0};
		v2 mouseMotion = {0,0};

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
            ImGui_ImplSdlGL3_ProcessEvent(&event);
			winHandleEvent(&w, &event);
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		winUpdateScrolling(&w);

		//glBindFramebuffer(GL_FRAMEBUFFER, gameScreen.glHandle);
		//glViewport(0,0,gameScreen.color.size.w, gameScreen.color.size.h);
		winClear(0,0,0);

		if (!io.WantCaptureMouse)
		{
			editorMoveCamera(&e, &w.scrollX.frame_scroll_amount, &w.scrollY.frame_scroll_amount);
			if (w.scrollX.frame_scroll_amount == 0)
				w.scrollX.scrolling = 0;
			if (w.scrollY.frame_scroll_amount == 0)
				w.scrollY.scrolling = 0;
		}
		editorUpdateCamera(&e);

		m4 la = MakeLookat(e.camPt + e.camPos, e.camUp, e.camPt);
		m4 ps = MakePerspective(e.vfov, (float)w.size.w/(float)w.size.h, 0.5f, 120.0f);
		m4 pv = ps*la;

		v2i mousePos;
		uint32_t mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);
		v2 ndcMousePos = winGetNormalizedScreenPoint(&w, mousePos);

		bool mouseIsDown = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && !io.WantCaptureMouse;
		bool mousePressed = mouseIsDown && !e.mouseWasDown;
		bool mouseReleased = !mouseIsDown && e.mouseWasDown;
		e.mouseWasDown = mouseIsDown;

		//game->physWorld.dynamicsWorld->stepSimulation(1 / 60.f, 10);



		obj3d_editor *hoveringObj = NULL;
		if (e.editState == 0 && e.objHandle.state == manipulator_states::GRABBABLE)
		{
			//can pick objects here too
			int indexOfMinDist = -1;
			float minDistToPick = FLT_MAX;
			for (int i = 0; i < e.numObjects; i++)
			{
				obj3d_editor *edit = &e.objects[i];
				v2 screenPos = WorldPointToNDCScreenPoint(edit->position, pv);
				float distToPick = LengthSq(screenPos - ndcMousePos);
				if (distToPick < minDistToPick)
				{
					minDistToPick = distToPick;
					indexOfMinDist = i;
				}
			}

			if (indexOfMinDist >= 0 && minDistToPick < 0.005f)
				hoveringObj = &e.objects[indexOfMinDist];
		}


		glBindFramebuffer(GL_FRAMEBUFFER, outlineDrawScreen.glHandle);
		glViewport(0,0,outlineDrawScreen.color.size.w, outlineDrawScreen.color.size.h);
		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);

		if (e.edit || hoveringObj)
		{
			glDisable(GL_DEPTH_TEST);
			glUseProgram(game->ass.basicMeshSolidColor.glHandle);
			GLint perspView = glGetUniformLocation(game->ass.basicMeshSolidColor.glHandle, "PerspView");
			GLint modelMat = glGetUniformLocation(game->ass.basicMeshSolidColor.glHandle, "ModelMat");
			GLint meshColor = glGetUniformLocation(game->ass.basicMeshSolidColor.glHandle, "MeshColor");
			glUniformMatrix4fv(perspView, 1, false, &pv[0]);

			if (e.edit)
			{
				glUniformMatrix4fv(modelMat, 1, false, &e.edit->modelMat[0]);
				v4 col = V4(1, 0, 0, 1);
				glUniform4fv(meshColor, 1, (float *)&col.x);
				if (e.edit->curVao)
					rendRenderVAO(e.edit->curVao);
				else
					rendRenderVAO(&game->ass.objmarker.mesh);
			}
			if (hoveringObj)
			{
				glUniformMatrix4fv(modelMat, 1, false, &hoveringObj->modelMat[0]);
				v4 col = V4(0, 1, 0, 1);
				glUniform4fv(meshColor, 1, (float *)&col.x);
				if (hoveringObj->curVao)
					rendRenderVAO(hoveringObj->curVao);
				else
					rendRenderVAO(&game->ass.objmarker.mesh);
			}

			glEnable(GL_DEPTH_TEST);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0, w.backingRes.w, w.backingRes.h);

		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Draw all the objects
		for (int i = 0; i < e.numObjects; i++)
		{
			obj3d_editor *obj = &e.objects[i];
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

			glUseProgram(game->ass.basicMeshColoredVerts.glHandle);
			GLint perspView = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "PerspView");
			GLint modelMat = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "ModelMat");
			glUniformMatrix4fv(perspView, 1, false, &pv[0]);
			glUniformMatrix4fv(modelMat, 1, false, &obj->modelMat[0]);
			if (obj->curVao)
				rendRenderVAO(obj->curVao);
			else
				rendRenderVAO(&game->ass.objmarker.mesh);
		}


		const Uint8 *state = SDL_GetKeyboardState(NULL);
		bool escIsDown = state[SDL_SCANCODE_ESCAPE] && !io.WantCaptureKeyboard;
		bool escPressed = escIsDown & !e.escWasDown;
		e.escWasDown = escIsDown;

		bool gIsDown = state[SDL_SCANCODE_G] && !io.WantCaptureKeyboard;
		bool gPressed = gIsDown & !e.gWasDown;
		e.gWasDown = gIsDown;

		bool lIsDown = state[SDL_SCANCODE_L] && !io.WantCaptureKeyboard;
		bool lPressed = lIsDown & !e.lWasDown;
		e.lWasDown = lIsDown;

		bool sIsDown = state[SDL_SCANCODE_S] && !io.WantCaptureKeyboard;
		bool sPressed = sIsDown & !e.sWasDown;
		e.sWasDown = sIsDown;

		bool tIsDown = state[SDL_SCANCODE_T] && !io.WantCaptureKeyboard;
		bool tPressed = tIsDown & !e.tWasDown;
		e.tWasDown = tIsDown;

		bool rIsDown = state[SDL_SCANCODE_R] && !io.WantCaptureKeyboard;
		bool rPressed = rIsDown & !e.rWasDown;
		e.rWasDown = rIsDown;

		bool xIsDown = state[SDL_SCANCODE_X] && !io.WantCaptureKeyboard;
		bool xPressed = xIsDown & !e.xWasDown;
		e.xWasDown = xIsDown;

		bool yIsDown = state[SDL_SCANCODE_Y] && !io.WantCaptureKeyboard;
		bool yPressed = yIsDown & !e.yWasDown;
		e.yWasDown = yIsDown;

		bool zIsDown = state[SDL_SCANCODE_Z] && !io.WantCaptureKeyboard;
		bool zPressed = zIsDown & !e.zWasDown;
		e.zWasDown = zIsDown;

		if (escPressed)
		{
			if (e.editState != 0)
				e.editState = 0;
			else
				e.showDebug = !e.showDebug;
		}

		if (gPressed)
			e.localHandle = 0;
		if (lPressed)
			e.localHandle = 1;
		if (tPressed)
			e.editState = 0;
		if (rPressed)
			e.editState = 1;
		if (sPressed)
			e.editState = 2;
		

		bool axisDraw[3] = {true, true, true};
		bool axisHidden[3] = {false, false, false};
		bool planeDraw[3] = {false, false, false};

		//always scale in a local direction
		if (e.editState == 2)
			e.localHandle = 1;

		//Draw the physics shape
		if (e.edit)
		{
			if (e.edit->physType == (int)object3d_physics::DYNAMIC || e.edit->physType == (int)object3d_physics::KINETIC)
			{
				m4 model = RotationPositionToM4(e.edit->rotation, e.edit->position);

				glUseProgram(game->ass.basicMeshColoredVerts.glHandle);
				GLint perspView = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "PerspView");
				GLint modelMat = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "ModelMat");
				glUniformMatrix4fv(perspView, 1, false, &pv[0]);
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

				switch ((physics_shapes)e.edit->shape)
				{
					case SHAPE_PLANE_X:
					{
						m4 rotate = QuatToM4(CreateQuat(90, V3(0,1,0)));
						model = model * MakeTranslation(e.edit->offset.x, 0, 0);
						model = model * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapePlane.mesh);
					} break;
					case SHAPE_PLANE_Y:
					{
						m4 rotate = QuatToM4(CreateQuat(90, V3(-1,0,0)));
						model = model * MakeTranslation(0, e.edit->offset.x, 0);
						model = model * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapePlane.mesh);
					} break;
					case SHAPE_PLANE_Z:
					{
						model = model * MakeTranslation(0, 0, e.edit->offset.x);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapePlane.mesh);
					} break;
					case SHAPE_CAPSULE_X:
					{
						m4 rotate = QuatToM4(CreateQuat(90, V3(0,1,0)));
						m4 modelBottom = model * MakeTranslation(e.edit->offset.x - e.edit->dimensions.y * 0.5f, e.edit->offset.y, e.edit->offset.z);
						modelBottom = modelBottom * MakeScale(e.edit->dimensions.x, e.edit->dimensions.x, e.edit->dimensions.x);
						modelBottom = modelBottom * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelBottom[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleBottom.mesh);

						m4 modelMiddle = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						modelMiddle = modelMiddle * MakeScale(e.edit->dimensions.y, e.edit->dimensions.x, e.edit->dimensions.x);
						modelMiddle = modelMiddle * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelMiddle[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleMiddle.mesh);

						m4 modelTop = model * MakeTranslation(e.edit->offset.x + e.edit->dimensions.y * 0.5f, e.edit->offset.y, e.edit->offset.z);
						modelTop = modelTop * MakeScale(e.edit->dimensions.x, e.edit->dimensions.x, e.edit->dimensions.x);
						modelTop = modelTop * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelTop[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleTop.mesh);
					} break;
					case SHAPE_CAPSULE_Y:
					{
						m4 rotate = QuatToM4(CreateQuat(90, V3(-1,0,0)));
						m4 modelBottom = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y - e.edit->dimensions.y * 0.5f, e.edit->offset.z);
						modelBottom = modelBottom * MakeScale(e.edit->dimensions.x, e.edit->dimensions.x, e.edit->dimensions.x);
						modelBottom = modelBottom * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelBottom[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleBottom.mesh);

						m4 modelMiddle = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						modelMiddle = modelMiddle * MakeScale(e.edit->dimensions.x, e.edit->dimensions.y, e.edit->dimensions.x);
						modelMiddle = modelMiddle * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelMiddle[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleMiddle.mesh);

						m4 modelTop = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y + e.edit->dimensions.y * 0.5f, e.edit->offset.z);
						modelTop = modelTop * MakeScale(e.edit->dimensions.x, e.edit->dimensions.x, e.edit->dimensions.x);
						modelTop = modelTop * rotate;
						glUniformMatrix4fv(modelMat, 1, false, &modelTop[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleTop.mesh);
					} break;
					case SHAPE_CAPSULE_Z:
					{
						m4 modelBottom = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z - e.edit->dimensions.y * 0.5f);
						modelBottom = modelBottom * MakeScale(e.edit->dimensions.x, e.edit->dimensions.x, e.edit->dimensions.x);
						glUniformMatrix4fv(modelMat, 1, false, &modelBottom[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleBottom.mesh);

						m4 modelMiddle = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						modelMiddle = modelMiddle * MakeScale(e.edit->dimensions.x, e.edit->dimensions.x, e.edit->dimensions.y);
						glUniformMatrix4fv(modelMat, 1, false, &modelMiddle[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleMiddle.mesh);

						m4 modelTop = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z + e.edit->dimensions.y * 0.5f);
						modelTop = modelTop * MakeScale(e.edit->dimensions.x, e.edit->dimensions.x, e.edit->dimensions.x);
						glUniformMatrix4fv(modelMat, 1, false, &modelTop[0]);
						rendRenderVAO(&game->ass.physShapeCapsuleTop.mesh);
					} break;
					case SHAPE_CONE_X:
					{
						model = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						model = model * MakeScale(e.edit->dimensions.y, e.edit->dimensions.x, e.edit->dimensions.x);
						model = model * QuatToM4(CreateQuat(90, V3(0,1,0)));
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCone.mesh);
					} break;
					case SHAPE_CONE_Y:
					{
						model = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						model = model * MakeScale(e.edit->dimensions.x, e.edit->dimensions.y, e.edit->dimensions.x);
						model = model * QuatToM4(CreateQuat(90, V3(-1,0,0)));
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCone.mesh);
					} break;
					case SHAPE_CONE_Z:
					{
						model = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						model = model * MakeScale(e.edit->dimensions.x, e.edit->dimensions.x, e.edit->dimensions.y);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCone.mesh);
					} break;
					case SHAPE_SPHERE:
					{
						model = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						model = model * MakeScale(e.edit->dimensions.x, e.edit->dimensions.x, e.edit->dimensions.x);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeSphere.mesh);
					} break;
					case SHAPE_CYLINDER_X:
					{
						model = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						model = model * MakeScale(e.edit->dimensions.y, e.edit->dimensions.x, e.edit->dimensions.x);
						model = model * QuatToM4(CreateQuat(90, V3(0,1,0)));
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCylinder.mesh);
					} break;
					case SHAPE_CYLINDER_Y:
					{
						model = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						model = model * MakeScale(e.edit->dimensions.x, e.edit->dimensions.y, e.edit->dimensions.x);
						model = model * QuatToM4(CreateQuat(90, V3(1,0,0)));
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCylinder.mesh);
					} break;
					case SHAPE_CYLINDER_Z:
					{
						model = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						model = model * MakeScale(e.edit->dimensions.x, e.edit->dimensions.x, e.edit->dimensions.y);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeCylinder.mesh);
					} break;
					case SHAPE_BOX:
					{
						model = model * MakeTranslation(e.edit->offset.x, e.edit->offset.y, e.edit->offset.z);
						model = model * MakeScale(e.edit->dimensions.x, e.edit->dimensions.y, e.edit->dimensions.z);
						glUniformMatrix4fv(modelMat, 1, false, &model[0]);
						rendRenderVAO(&game->ass.physShapeRectangularPrism.mesh);
					} break;
				}
				glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			}


			if (e.editState == 0)
			{
				e.objRotator.state = rotator_states::PICK_AXIS;
				switch (e.objHandle.state)
				{
					case manipulator_states::GRABBABLE:
					{
						v3 xAxisDir = V3(2,0,0);
						v3 yAxisDir = V3(0,2,0);
						v3 zAxisDir = V3(0,0,2);
						if (e.localHandle)
						{
							xAxisDir = RotateByQuat(xAxisDir, e.edit->rotation);
							yAxisDir = RotateByQuat(yAxisDir, e.edit->rotation);
							zAxisDir = RotateByQuat(zAxisDir, e.edit->rotation);
						}
						e.objHandle.xAxisHandle = {WorldPointToNDCScreenPoint(e.edit->position, pv), WorldPointToNDCScreenPoint(e.edit->position + xAxisDir, pv)};
						e.objHandle.yAxisHandle = {WorldPointToNDCScreenPoint(e.edit->position, pv), WorldPointToNDCScreenPoint(e.edit->position + yAxisDir, pv)};
						e.objHandle.zAxisHandle = {WorldPointToNDCScreenPoint(e.edit->position, pv), WorldPointToNDCScreenPoint(e.edit->position + zAxisDir, pv)};

						float distToXHandle = DistanceToLineSeg2D(e.objHandle.xAxisHandle, ndcMousePos, NULL);
						float distToYHandle = DistanceToLineSeg2D(e.objHandle.yAxisHandle, ndcMousePos, NULL);
						float distToZHandle = DistanceToLineSeg2D(e.objHandle.zAxisHandle, ndcMousePos, NULL);

						bool canManipulate = false;
						if (distToXHandle <= distToYHandle && distToXHandle <= distToZHandle)
						{
							if (distToXHandle < 0.05f)
							{
								canManipulate = true;
								e.objHandle.whichAxis = 0;
								e.objHandle.axisLine = {e.edit->position, e.edit->position + xAxisDir};
							}
						}
						if (distToYHandle <= distToXHandle && distToYHandle <= distToZHandle)
						{
							if (distToYHandle < 0.05f)
							{
								canManipulate = true;
								e.objHandle.whichAxis = 1;
								e.objHandle.axisLine = {e.edit->position, e.edit->position + yAxisDir};
							}
						}
						if (distToZHandle <= distToXHandle && distToZHandle <= distToYHandle)
						{
							if (distToZHandle < 0.05f)
							{
								canManipulate = true;
								e.objHandle.whichAxis = 2;
								e.objHandle.axisLine = {e.edit->position, e.edit->position + zAxisDir};
							}
						}

						if (canManipulate)
						{
							axisDraw[e.objHandle.whichAxis] = true;
							int otherA = 0, otherB = 1;
							if (e.objHandle.whichAxis == 0)
								otherA = 2;
							else if (e.objHandle.whichAxis == 1)
								otherB = 2;
							axisDraw[otherA] = false;
							axisDraw[otherB] = false;
							axisHidden[otherA] = true;
							axisHidden[otherB] = true;
						}

						if (canManipulate && mousePressed && !io.WantCaptureMouse)
						{
							e.objHandle.state = manipulator_states::IS_MANIPULATING;

							v2 pointOnAxis;
							DistanceToLine(e.objHandle.axisHandle[e.objHandle.whichAxis], ndcMousePos, &pointOnAxis);
							line_seg_3d mouseLine = WindowPointToWorldLineSeg(pointOnAxis, pv);

							bool linesParallel;
							v3 closestPtA, closestPtB;
							ClosestPointsOn3DLines(e.objHandle.axisLine, mouseLine, &closestPtA, &closestPtB, &linesParallel);

							if (!linesParallel)
								e.objHandle.holdingOffset = closestPtA - e.edit->position;
							else
								e.objHandle.state = manipulator_states::GRABBABLE;
						}
						if (!canManipulate && mousePressed && hoveringObj)
						{
							e.edit = hoveringObj;
						}

					} break;

					case manipulator_states::IS_MANIPULATING:
					{
						axisDraw[e.objHandle.whichAxis] = true;
						int otherA = 0, otherB = 1;
						if (e.objHandle.whichAxis == 0)
							otherA = 2;
						else if (e.objHandle.whichAxis == 1)
							otherB = 2;
						axisDraw[otherA] = false;
						axisDraw[otherB] = false;
						axisHidden[otherA] = true;
						axisHidden[otherB] = true;

						v2 pointOnAxis;
						DistanceToLine(e.objHandle.axisHandle[e.objHandle.whichAxis], ndcMousePos, &pointOnAxis);
						line_seg_3d mouseLine = WindowPointToWorldLineSeg(pointOnAxis, pv);

						bool linesParallel;
						v3 closestPtA, closestPtB;
						ClosestPointsOn3DLines(e.objHandle.axisLine, mouseLine, &closestPtA, &closestPtB, &linesParallel);

						if (!linesParallel)
						{
							e.edit->position = closestPtA - e.objHandle.holdingOffset;
							if (e.snapToGrid != 0)
							{
								if (e.localHandle == 0)
								{
									float *vals = (float *)&e.edit->position.x;
									float valForAxis = vals[e.objHandle.whichAxis];
									vals[e.objHandle.whichAxis] = SnapToGrid(valForAxis, e.snapToGrid);
								}
								else
								{
									e.edit->position.x = SnapToGrid(e.edit->position.x, e.snapToGrid);
									e.edit->position.y = SnapToGrid(e.edit->position.y, e.snapToGrid);
									e.edit->position.z = SnapToGrid(e.edit->position.z, e.snapToGrid);
								}
							}
						}

						if (mouseReleased)
							e.objHandle.state = manipulator_states::GRABBABLE;
					} break;
				}
			}
			else if (e.editState == 1)
			{
				switch (e.objRotator.state)
				{
					case rotator_states::PICK_AXIS:
					{
						planeDraw[0] = true; planeDraw[1] = true; planeDraw[2] = true;

						m4 model;
						if (e.localHandle)
							model = QuatToM4(e.edit->rotation);
						else
							model = M4();
						model.col[3] = V4(e.edit->position.x, e.edit->position.y, e.edit->position.z, 1.0f);

						e.objRotator.rotationAmount = 0;
						if (xPressed)
						{
							e.objRotator.initialRotation = e.edit->rotation;
							e.objRotator.whichAxis = 0;
							e.objRotator.state = rotator_states::AXIS_PICKED;
						}
						else if (yPressed)
						{
							e.objRotator.initialRotation = e.edit->rotation;
							e.objRotator.whichAxis = 1;
							e.objRotator.state = rotator_states::AXIS_PICKED;
						}
						else if (zPressed)
						{
							e.objRotator.initialRotation = e.edit->rotation;
							e.objRotator.whichAxis = 2;
							e.objRotator.state = rotator_states::AXIS_PICKED;
						}
					} break;

					case rotator_states::AXIS_PICKED:
					{
						axisDraw[e.objRotator.whichAxis] = true;
						planeDraw[e.objRotator.whichAxis] = true;
						int otherA = 0, otherB = 1;
						if (e.objRotator.whichAxis == 0)
							otherA = 2;
						else if (e.objRotator.whichAxis == 1)
							otherB = 2;
						axisDraw[otherA] = false;
						axisDraw[otherB] = false;
						axisHidden[otherA] = true;
						axisHidden[otherB] = true;

						if (e.objRotator.rotationAmount != 0)
						{
							v3 normal = V3(1,0,0);
							if (e.objRotator.whichAxis == 1)
								normal = V3(0,1,0);
							else if (e.objRotator.whichAxis == 2)
								normal = V3(0,0,1);
							if (e.localHandle)
								normal = RotateByQuat(normal, e.objRotator.initialRotation);
							
							e.edit->rotation = RotateQuatByQuat(e.objRotator.initialRotation, CreateQuat(e.objRotator.rotationAmount, normal));
						}
						else
							e.edit->rotation = e.objRotator.initialRotation;

						if (mousePressed && !io.WantCaptureMouse)
						{
							e.objRotator.initialRotation = e.edit->rotation;
							e.objRotator.rotationAmount = 0;

							v3 normal;
							v3 up;
							v3 right;
							if (e.objRotator.whichAxis == 0) //x
							{
								normal = V3(1,0,0);
								up = V3(0,1,0);
								right = V3(0,0,-1);
							}
							if (e.objRotator.whichAxis == 1) //y
							{
								normal = V3(0,1,0);
								up = V3(0,0,1);
								right = V3(-1,0,0);
							}
							if (e.objRotator.whichAxis == 2) //z
							{
								normal = V3(0,0,1);
								up = V3(1,0,0);
								right = V3(0,-1,0);
							}

							if (e.localHandle)
							{
								normal = RotateByQuat(normal, e.objRotator.initialRotation);
								up = RotateByQuat(up, e.objRotator.initialRotation);
								right = RotateByQuat(right, e.objRotator.initialRotation);
							}

							plane rotatePlane = PlaneFromNormalAndPoint(normal, e.edit->position);

							e.objRotator.initialUp = up;
							e.objRotator.initialRight = right;
							e.objRotator.initialNormal = normal;
							e.objRotator.rotatePlane = rotatePlane;

							line_seg_3d mouseLine = WindowPointToWorldLineSeg(ndcMousePos, pv);
							bool parallel;
							v3 intersection;
							LinePlaneIntersection(mouseLine, rotatePlane, &intersection, &parallel);

							if (!parallel)
							{
								v3 dragVector = intersection - e.edit->position;
								//e.objRotator.initialRotation = e.edit->rotation;
								v2 rotationPt = {Inner(right, dragVector), Inner(up, dragVector)};
								e.objRotator.rotationOffset = atan2f(rotationPt.y, rotationPt.x);
								e.objRotator.state = rotator_states::IS_ROTATING;
							}
						}

						const Uint8 *state = SDL_GetKeyboardState(NULL);
						if (state[SDL_SCANCODE_X])
						{
							e.objRotator.whichAxis = 0;
							e.objRotator.rotationAmount = 0;
							e.objRotator.initialRotation = e.edit->rotation;
						}
						else if (state[SDL_SCANCODE_Y])
						{
							e.objRotator.whichAxis = 1;
							e.objRotator.rotationAmount = 0;
							e.objRotator.initialRotation = e.edit->rotation;
						}
						else if (state[SDL_SCANCODE_Z])
						{
							e.objRotator.whichAxis = 2;
							e.objRotator.rotationAmount = 0;
							e.objRotator.initialRotation = e.edit->rotation;
						}
					} break;

					case rotator_states::IS_ROTATING:
					{
						axisDraw[e.objRotator.whichAxis] = true;
						planeDraw[e.objRotator.whichAxis] = true;
						int otherA = 0, otherB = 1;
						if (e.objRotator.whichAxis == 0)
							otherA = 2;
						else if (e.objRotator.whichAxis == 1)
							otherB = 2;
						axisDraw[otherA] = false;
						axisDraw[otherB] = false;
						axisHidden[otherA] = true;
						axisHidden[otherB] = true;


						line_seg_3d mouseLine = WindowPointToWorldLineSeg(ndcMousePos, pv);
						bool parallel;
						v3 intersection;
						LinePlaneIntersection(mouseLine, e.objRotator.rotatePlane, &intersection, &parallel);

						if (!parallel)
						{
							v3 dragVector = intersection - e.edit->position;
							v2 rotationPt = {Inner(e.objRotator.initialRight, dragVector), Inner(e.objRotator.initialUp, dragVector)};
							float rotation = atan2f(rotationPt.y, rotationPt.x);
							e.objRotator.rotationAmount = rotation - e.objRotator.rotationOffset;
							e.objRotator.rotationAmount *= RADIANS_TO_DEGREES;
							WrapToRange(0.0f, &e.objRotator.rotationAmount, 360.0f);
							if (e.snapToAngle != 0)
								e.objRotator.rotationAmount = SnapToGrid(e.objRotator.rotationAmount, e.snapToAngle);
							quat newRotation = CreateQuatRAD(e.objRotator.rotationAmount * DEGREES_TO_RADIANS, e.objRotator.initialNormal);
							e.edit->rotation = RotateQuatByQuat(e.objRotator.initialRotation, newRotation);
						}

						if (mouseReleased)
						{
							e.objRotator.state = rotator_states::AXIS_PICKED;
							e.objRotator.initialRotation = e.edit->rotation;
							e.objRotator.rotationAmount = 0;
						}
					} break;
				}
			}
			else if (e.editState == 2) //scaling
			{
				e.objRotator.state = rotator_states::PICK_AXIS;
				axisDraw[0] = false; axisDraw[1] = false; axisDraw[2] = false;
				axisHidden[0] = true; axisHidden[1] = true; axisHidden[2] = true;
			}
		}
		
		if (e.edit)
		{
			m4 model;
			if (e.localHandle)
				model = QuatToM4(e.edit->rotation);
			else
				model = M4();
			model.col[3] = V4(e.edit->position.x, e.edit->position.y, e.edit->position.z, 1.0f);

			glUseProgram(game->ass.basicMeshColoredVerts.glHandle);
			GLint perspView = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "PerspView");
			GLint modelMat = glGetUniformLocation(game->ass.basicMeshColoredVerts.glHandle, "ModelMat");
			glUniformMatrix4fv(perspView, 1, false, &pv[0]);
			glUniformMatrix4fv(modelMat, 1, false, &model[0]);

			if (axisDraw[0])
				rendRenderVAO(&game->ass.manipulatorX.mesh);
			if (axisDraw[1])
				rendRenderVAO(&game->ass.manipulatorY.mesh);
			if (axisDraw[2])
				rendRenderVAO(&game->ass.manipulatorZ.mesh);

			if (planeDraw[0])
				rendRenderVAO(&game->ass.xaxis.mesh);
			if (planeDraw[1])
				rendRenderVAO(&game->ass.yaxis.mesh);
			if (planeDraw[2])
				rendRenderVAO(&game->ass.zaxis.mesh);

			glUseProgram(game->ass.hiddenMeshColoredVerts.glHandle);
			perspView = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "PerspView");
			modelMat = glGetUniformLocation(game->ass.hiddenMeshColoredVerts.glHandle, "ModelMat");
			glUniformMatrix4fv(perspView, 1, false, &pv[0]);
			glUniformMatrix4fv(modelMat, 1, false, &model[0]);
			glDepthFunc(GL_GREATER);
			if (axisDraw[0])
				rendRenderVAO(&game->ass.manipulatorX.mesh);
			if (axisDraw[1])
				rendRenderVAO(&game->ass.manipulatorY.mesh);
			if (axisDraw[2])
				rendRenderVAO(&game->ass.manipulatorZ.mesh);

			glDepthFunc(GL_ALWAYS);
			if (axisHidden[0])
				rendRenderVAO(&game->ass.manipulatorX.mesh);
			if (axisHidden[1])
				rendRenderVAO(&game->ass.manipulatorY.mesh);
			if (axisHidden[2])
				rendRenderVAO(&game->ass.manipulatorZ.mesh);
			glDepthFunc(GL_LESS);
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

		if (e.showDebug)
        {
			ImGui_ImplSdlGL3_NewFrame(w.win);

			static bool show_demo_window = true;
			ImGui::ShowDemoWindow(&show_demo_window);

            static int counter = 0;
            ImGui::SliderFloat("v-FOV", &e.vfov, 30.0f, 100.0f);
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::RadioButton("global", &e.localHandle, 0); ImGui::SameLine();
            ImGui::RadioButton("local", &e.localHandle, 1);

            ImGui::RadioButton("translate", &e.editState, 0); ImGui::SameLine();
            ImGui::RadioButton("rotate", &e.editState, 1); ImGui::SameLine();
            ImGui::RadioButton("scale", &e.editState, 2);
			
			if (e.edit)
			{
				ImGui::PushItemWidth(65);
				ImGui::InputFloat("grid", &e.snapToGrid); ImGui::SameLine();
				ImGui::InputFloat("angle", &e.snapToAngle);
				ImGui::PopItemWidth();

				if (e.editState == 0)
					ImGui::DragFloat3("tx", (float *)&e.edit->position.x, 0.01f);
				if (e.editState == 1)
				{
					if (e.objRotator.state == rotator_states::AXIS_PICKED || 
						e.objRotator.state == rotator_states::IS_ROTATING)
					{
						ImGui::InputFloat("amount", &e.objRotator.rotationAmount);
					}
					if (ImGui::Button("Reset rotation"))
					{
						e.objRotator.initialRotation = CreateQuat(0, V3(1,0,0));
						e.objRotator.rotationAmount = 0;
						e.edit->rotation = e.objRotator.initialRotation;
					}
				}
				if (e.editState == 2)
					ImGui::DragFloat3("scale", (float *)&e.edit->scale.x, 0.01f);
			}

			if (ImGui::Button("New"))
				editorNewObj3D(&e);

			if (e.edit)
			{
				ImGui::SameLine();
				if (ImGui::Button("Duplicate"))
				{

				}
				ImGui::SameLine();
				if (ImGui::Button("Delete"))
				{

				}

				ImGui::Checkbox("Starts Active", &e.edit->startsActive);
				ImGui::Checkbox("Starts Visible", &e.edit->startsVisible);
				ImGui::RadioButton("non-visual", &e.edit->objType, 0); ImGui::SameLine();
				ImGui::RadioButton("bone", &e.edit->objType, 1); ImGui::SameLine();
				ImGui::RadioButton("mesh", &e.edit->objType, 2);
				ImGui::RadioButton("non-physical", &e.edit->physType, 0); ImGui::SameLine();
				ImGui::RadioButton("dynamic", &e.edit->physType, 1); ImGui::SameLine();
				ImGui::RadioButton("kinetic", &e.edit->physType, 2);

				if (e.edit->physType == (int)object3d_physics::DYNAMIC || e.edit->physType == (int)object3d_physics::KINETIC)
				{
					ImGui::Combo("phys shape", &e.edit->shape, "PLANE_X\0PLANE_Y\0PLANE_Z\0CAPSULE_X\0CAPSULE_Y\0CAPSULE_Z\0CONE_X\0CONE_Y\0CONE_Z\0SPHERE\0CYLINDER_X\0CYLINDER_Y\0CYLINDER_Z\0BOX\0\0");
					switch ((physics_shapes)e.edit->shape)
					{
						case SHAPE_PLANE_X:
						case SHAPE_PLANE_Y:
						case SHAPE_PLANE_Z:
						{
							ImGui::DragFloat("offset", (float *)&e.edit->offset.x, 0.01f);
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
							ImGui::DragFloat("radius", (float *)&e.edit->dimensions.x, 0.01f);
							ImGui::DragFloat("height", (float *)&e.edit->dimensions.y, 0.01f);
							ImGui::DragFloat3("offset", (float *)&e.edit->offset.x, 0.01f);
							ImGui::DragFloat("mass", &e.edit->mass, 0.01f, 0.0f, 500.0f);
						} break;
						case SHAPE_SPHERE:
						{
							ImGui::DragFloat("radius", (float *)&e.edit->dimensions.x, 0.01f);
							ImGui::DragFloat3("offset", (float *)&e.edit->offset.x, 0.01f);
							ImGui::DragFloat("mass", &e.edit->mass, 0.01f, 0.0f, 500.0f);
						} break;
						case SHAPE_BOX:
						{
							ImGui::DragFloat3("dimensions", (float *)&e.edit->dimensions.x, 0.01f);
							ImGui::DragFloat3("offset", (float *)&e.edit->offset.x, 0.01f);
							ImGui::DragFloat("mass", &e.edit->mass, 0.01f, 0.0f, 500.0f);
						} break;
					}
					ImGui::DragFloat("restitution", &e.edit->restitution, 0.01f, 0.0f, 1.0f);
				}

				if (e.edit->numLODs < ArrayCount(e.edit->lods))
				{
					if (ImGui::Button("Add LOD mesh"))
						e.edit->numLODs++;
				}

				for (int i = 0; i < e.edit->numLODs; i++)
				{
					obj3d_mesh_assignment *lodMesh = &e.edit->lods[i];
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

#include "game_math.cpp"
#include "window.cpp"
#include "opengl.cpp"
#include "file_utilities.cpp"
#include "memory_arena.cpp"
#include "xml.cpp"
#include "collada.cpp"
#include "mesh.cpp"
#include "object3d.cpp"
#include "render.cpp"
#include "game_world.cpp"
#include "assets.cpp"
#include "physics.cpp"
#include "imgui_demo.cpp"
#include "imgui_draw.cpp"
#include "imgui.cpp"
#include "imgui_impl_sdl_gl3.cpp"
#include "hash.cpp"
#include "editor.cpp"
