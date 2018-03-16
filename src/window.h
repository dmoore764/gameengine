#pragma once

#include <SDL2/SDL.h>
#include "game_math.h"

struct inertial_scroll
{
	int    scrolling;					// flag (scrolling or not)
	int    scroll_sensitivity;			// how fast we want to scroll
	float frame_scroll_amount;
	float scroll_acceleration;			// scrolling speed
	float scroll_friction;				// how fast we decelerate
	float scroll_prev_pos;				// previous event's position
};

struct window
{
	SDL_Window *win;
	v2i size;
	v2i backingRes;
	SDL_GLContext glContext;
	union
	{
		struct
		{
			inertial_scroll scrollX;
			inertial_scroll scrollY;
		};
		inertial_scroll scroll[2];
	};
};

void winCreate(window *w, v2i size);
void winUpdateScrolling(window *w);
void winHandleEvent(window *w, SDL_Event *event);
void winClear(float r, float g, float b);
void winSwapBuffers(window *w);
void winClose(window *w);
v2 winGetNormalizedScreenPoint(window *w, v2i point);
