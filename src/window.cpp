#include "window.h"
#include "opengl.h"

void WinCreate(window *w, v2i size)
{
	w->size = size;
    SDL_Init(SDL_INIT_VIDEO);
    w->win = SDL_CreateWindow(
        "Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        size.w,
        size.h,
        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS
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

void WinClear(float r, float g, float b)
{
	glClearColor(r,g,b,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void WinSwapBuffers(window *w)
{
	SDL_GL_SwapWindow(w->win);
}

void WinClose(window *w)
{
	SDL_GL_DeleteContext(w->glContext); 
    SDL_DestroyWindow(w->win);
	w->win = NULL;
    SDL_Quit();
}
