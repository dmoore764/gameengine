#include "render.h"
#include "compiler_helper.h"

void rendSetTexture(texture *t, uint32_t gameTime, int slot)
{
	t->lastUsed = gameTime;
	if (!(t->loadFlags & TEX_LOADED))
	{
		oglLoadTexture2DIntoGPU(t);
	}

	if ((t->flags & (MIN_FILTERING_2_MIPMAPS_LINEAR | MIN_FILTERING_MIPMAP_LINEAR | MIN_FILTERING_MIPMAP_NEAREST | MIN_FILTERING_2_MIPMAPS_NEAREST)) && !(t->loadFlags & (TEX_MIPMAPS_GENERATED | TEX_MIPMAPS_LOADED)))
	{
		//trying to use a texture with no mipmaps, despite a min filtering that specifies it
		assert(false);
	}

	glActiveTexture(slot);

	if (t->flags & TEX_1D)
		glBindTexture(GL_TEXTURE_1D, t->glHandle);
	if (t->flags & TEX_1D_ARRAY)
		glBindTexture(GL_TEXTURE_1D_ARRAY, t->glHandle);
	if (t->flags & TEX_2D)
		glBindTexture(GL_TEXTURE_2D, t->glHandle);
	if (t->flags & TEX_2D_ARRAY)
		glBindTexture(GL_TEXTURE_2D_ARRAY, t->glHandle);
	if (t->flags & TEX_CUBEMAP)
		glBindTexture(GL_TEXTURE_CUBE_MAP, t->glHandle);
	if (t->flags & TEX_CUBEMAP_ARRAY)
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, t->glHandle);
	if (t->flags & TEX_2D_MULTISAMPLE)
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, t->glHandle);
	if (t->flags & TEX_2D_MULTISAMPLE_ARRAY)
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, t->glHandle);
}

void rendRenderVAO(vertex_array_object *vao)
{
	glBindVertexArray(vao->vaoHandle);
	
	switch (vao->type)
	{
		case VAO_DRAW_ARRAYS:
		{
			glDrawArrays(GL_TRIANGLES, 0, vao->numVertices);
		} break;

		case VAO_DRAW_ELEMENTS:
		{
			glDrawElements(GL_TRIANGLES, vao->numIndices, GL_UNSIGNED_INT, 0);
		} break;
	}
}
