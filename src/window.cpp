#include "window.h"
#include "opengl.h"

void winCreate(window *w, v2i size)
{
	w->size = size;
    SDL_Init(SDL_INIT_VIDEO);
    w->win = SDL_CreateWindow(
        "Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        size.w,
        size.h,
        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
    );

    if (w->win == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
		return;
    }

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetSwapInterval(1);
	w->glContext = SDL_GL_CreateContext(w->win);
	SDL_ShowWindow(w->win);

	SDL_GL_GetDrawableSize(w->win, &w->backingRes.w, &w->backingRes.h);
}

void winUpdateScrolling(window *w)
{
	for (int i = 0; i < ArrayCount(w->scroll); i++)
	{
		inertial_scroll *scroll = &w->scroll[i];
		if (scroll->scrolling)
		{
			if (scroll->scroll_acceleration > 0) 
				scroll->scroll_acceleration -= scroll->scroll_friction;
			if (scroll->scroll_acceleration < 0) 
				scroll->scroll_acceleration += scroll->scroll_friction;
			if (Abs(scroll->scroll_acceleration) < scroll->scroll_friction * 3) 
				scroll->scroll_acceleration = 0;
			scroll->frame_scroll_amount = scroll->scroll_sensitivity * scroll->scroll_acceleration;
		}
		else
			scroll->frame_scroll_amount = 0;
	}
}

void winHandleEvent(window *w, SDL_Event *event)
{
	if (event->type == SDL_MULTIGESTURE)
	{
		if (event->mgesture.numFingers == 2)
		{
			if (w->scrollX.scrolling == 0)
			{
				w->scrollX.scrolling = 1;
				w->scrollX.scroll_prev_pos = event->mgesture.x;
			}
			else
			{
				double dx = event->mgesture.x - w->scrollX.scroll_prev_pos;
				w->scrollX.scroll_acceleration = dx * 40;
				w->scrollX.scroll_prev_pos = event->mgesture.x;
				w->scrollX.scrolling = 1;
			}

			if (w->scrollY.scrolling == 0)
			{
				w->scrollY.scrolling = 1;
				w->scrollY.scroll_prev_pos = event->mgesture.y;
			}
			else
			{
				double dy = event->mgesture.y - w->scrollY.scroll_prev_pos;
				w->scrollY.scroll_acceleration = dy * 40;
				w->scrollY.scroll_prev_pos = event->mgesture.y;
				w->scrollY.scrolling = 1;
			}
		}
	} 

	if (event->type == SDL_FINGERDOWN)
	{
		w->scrollX.scrolling = 0;
		w->scrollY.scrolling = 0;
	}

	if (event->type == SDL_WINDOWEVENT) {
		switch (event->window.event) {
			case SDL_WINDOWEVENT_SHOWN:
			{
				//SDL_Log("Window %d shown", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_HIDDEN:
			{
				//SDL_Log("Window %d hidden", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_EXPOSED:
			{
				//SDL_Log("Window %d exposed", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_MOVED:
			{
				//SDL_Log("Window %d moved to %d,%d", event->window.windowID, event->window.data1, event->window.data2);
			} break;
			case SDL_WINDOWEVENT_RESIZED:
			{
				//SDL_Log("Window %d resized to %dx%d", event->window.windowID, event->window.data1, event->window.data2);
				w->size = V2I(event->window.data1, event->window.data2);
				SDL_GL_GetDrawableSize(w->win, &w->backingRes.w, &w->backingRes.h);
			} break;
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			{
				//SDL_Log("Window %d size changed to %dx%d", event->window.windowID, event->window.data1, event->window.data2);
				w->size = V2I(event->window.data1, event->window.data2);
				SDL_GL_GetDrawableSize(w->win, &w->backingRes.w, &w->backingRes.h);
			} break;
			case SDL_WINDOWEVENT_MINIMIZED:
			{
				//SDL_Log("Window %d minimized", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_MAXIMIZED:
			{
				//SDL_Log("Window %d maximized", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_RESTORED:
			{
				//SDL_Log("Window %d restored", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_ENTER:
			{
				//SDL_Log("Mouse entered window %d", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_LEAVE:
			{
				//SDL_Log("Mouse left window %d", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			{
				//SDL_Log("Window %d gained keyboard focus", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
			{
				//SDL_Log("Window %d lost keyboard focus", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_CLOSE:
			{
				//SDL_Log("Window %d closed", event->window.windowID);
			} break;

#if SDL_VERSION_ATLEAST(2, 0, 5)
			case SDL_WINDOWEVENT_TAKE_FOCUS:
			{
				//SDL_Log("Window %d is offered a focus", event->window.windowID);
			} break;
			case SDL_WINDOWEVENT_HIT_TEST:
			{
				//SDL_Log("Window %d has a special hit test", event->window.windowID);
			} break;
#endif
			default:
			{
				//SDL_Log("Window %d got unknown event %d", event->window.windowID, event->window.event);
			} break;
		}
    }
}

void winClear(float r, float g, float b)
{
	glClearColor(r,g,b,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void winSwapBuffers(window *w)
{
	SDL_GL_SwapWindow(w->win);
}

void winClose(window *w)
{
	SDL_GL_DeleteContext(w->glContext); 
    SDL_DestroyWindow(w->win);
	w->win = NULL;
    SDL_Quit();
}

v2 winGetNormalizedScreenPoint(window *w, v2i point)
{
	v2 result;
	result.x = 2 * ((float)point.x / (float)w->size.w) - 1;
	result.y = 2 * ((float)(w->size.h - point.y) / (float)w->size.h) - 1;
	return result;
}
