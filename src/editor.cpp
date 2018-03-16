#include "editor.h"
#include <SDL2/SDL.h>

void editorInit(editor *e)
{
	memset(e, '\0', sizeof(editor));
	e->camTheta = 30 * DEGREES_TO_RADIANS;
	e->camPhi = 10 * DEGREES_TO_RADIANS;
	e->camZoom = 8;
	e->camUp = V3(0,0,1);

	e->vfov = 50;
	e->showDebug = true;
	e->snapToAngle = 10.0f;
}

void editorNewObj3D(editor *e)
{
	obj3d_editor *result = &e->objects[e->numObjects++];
	memset(result, '\0', sizeof(obj3d_editor));
	result->editing = true;
	result->position = e->camPt;
	if (e->snapToGrid != 0)
	{
		result->position.x = SnapToGrid(result->position.x, e->snapToGrid);
		result->position.y = SnapToGrid(result->position.y, e->snapToGrid);
		result->position.z = SnapToGrid(result->position.z, e->snapToGrid);
	}
	result->rotation = CreateQuat(0, V3(1,0,0));
	result->scale = V3(1,1,1);
	result->startsActive = true;
	result->startsVisible = true;
	e->edit = result;
}

void editorUpdateCamera(editor *e)
{
	e->camPos = V3(e->camZoom * cos(e->camPhi), e->camZoom * sin(e->camPhi), 0);
	e->camPos = RotateByQuat(e->camPos, CreateQuatRAD(e->camTheta, Cross(e->camPos, e->camUp)));
}

void editorMoveCamera(editor *e, float *scrollVelX, float *scrollVelY)
{
	if (!(e->editState == 0 && e->objHandle.state == manipulator_states::IS_MANIPULATING) &&
			!(e->editState == 1 && e->objRotator.state == rotator_states::IS_ROTATING))
	{
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT])
		{
			v3 zaxis = Normalize(e->camPos);
			v3 xaxis = Normalize(Cross(e->camUp, zaxis));
			e->camPt = e->camPt + (*scrollVelX * 0.005f) * xaxis;
			e->camPt = e->camPt - (*scrollVelY * 0.005f) * e->camUp;
		}
		else if (state[SDL_SCANCODE_LGUI] || state[SDL_SCANCODE_RGUI])
		{
			e->camZoom = ClampToRange(1.0f, e->camZoom + *scrollVelY * e->camZoom * 0.006f, FLT_MAX);
		}
		else if (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL])
		{
			e->camPhi -= *scrollVelX * 0.005f;
			e->camTheta = ClampToRange(-PI / 2, e->camTheta + *scrollVelY * 0.006f, PI / 2);
		}

		local_persistent bool guiWasDown = false;
		bool guiIsDown = (state[SDL_SCANCODE_LGUI] || state[SDL_SCANCODE_RGUI]);
		bool guiReleased = !guiIsDown && guiWasDown;
		guiWasDown = guiIsDown;

		local_persistent bool shiftWasDown = false;
		bool shiftIsDown = (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]);
		bool shiftReleased = !shiftIsDown && shiftWasDown;
		shiftWasDown = shiftIsDown;

		local_persistent bool ctrlWasDown = false;
		bool ctrlIsDown = (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]);
		bool ctrlReleased = !ctrlIsDown && ctrlWasDown;
		ctrlWasDown = ctrlIsDown;

		local_persistent bool cWasDown = false;
		bool cIsDown = state[SDL_SCANCODE_C];
		bool cPressed = cIsDown && !cWasDown;
		cWasDown = cIsDown;

		if (ctrlReleased || shiftReleased || guiReleased)
		{
			*scrollVelX = 0;
			*scrollVelY = 0;
		}

		if (cPressed && e->edit)
			e->camPt = V3(e->edit->position.x, e->edit->position.y, e->camPt.z);
	}
}
