#pragma once
#include "opengl.h"

void rendSetTexture(texture *t, uint32_t gameTime, int slot = 0);
void rendRenderVAO(vertex_array_object *vao);
