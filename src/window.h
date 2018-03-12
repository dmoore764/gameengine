#pragma once

#include <SDL2/SDL.h>
#include "game_math.h"

struct window
{
	SDL_Window *win;
	v2i size;
	v2i backingRes;
	SDL_GLContext glContext;
};

void winCreate(window *w, v2i size);
void winHandleEvent(window *w, SDL_Event *event);
void winClear(float r, float g, float b);
void winSwapBuffers(window *w);
void winClose(window *w);
v2 winGetNormalizedScreenPoint(window *w, v2i point);
