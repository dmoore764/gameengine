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

void WinCreate(window *w, v2i size);
void WinClear(float r, float g, float b);
void WinSwapBuffers(window *w);
void WinClose(window *w);
