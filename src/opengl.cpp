#include "opengl.h"

#include "file_utilities.h"
#include "compiler_helper.h"
#include "stdlib.h"
#include "stddef.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

internal_function void _SetTexParameters(uint32_t flags)
{
	GLenum target = GL_TEXTURE_2D;

	if (flags & TEX_1D)
		target = GL_TEXTURE_1D;
	else if (flags & TEX_1D_ARRAY)
		target = GL_TEXTURE_1D_ARRAY;
	else if (flags & TEX_2D)
		target = GL_TEXTURE_2D;
	else if (flags & TEX_2D_ARRAY)
		target = GL_TEXTURE_2D_ARRAY;
	else if (flags & TEX_2D_MULTISAMPLE)
		target = GL_TEXTURE_2D_MULTISAMPLE;
	else if (flags & TEX_2D_MULTISAMPLE_ARRAY)
		target = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
	else if (flags & TEX_CUBEMAP)
		target = GL_TEXTURE_CUBE_MAP;
	else if (flags & TEX_CUBEMAP_ARRAY)
		target = GL_TEXTURE_CUBE_MAP_ARRAY;

	if (flags & MAG_FILTERING_NEAREST)
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	else if (flags & MAG_FILTERING_LINEAR)
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (flags & MIN_FILTERING_NEAREST)
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	else if (flags & MIN_FILTERING_LINEAR)
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	else if (flags & MIN_FILTERING_MIPMAP_NEAREST)
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	else if (flags & MIN_FILTERING_MIPMAP_LINEAR)
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	else if (flags & MIN_FILTERING_2_MIPMAPS_NEAREST)
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	else if (flags & MIN_FILTERING_2_MIPMAPS_LINEAR)
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	if (flags & CLAMP_TO_EDGE_S)
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	else if (flags & REPEAT_S)
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	else if (flags & MIRRORED_REPEAT_S)
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);

	if (flags & CLAMP_TO_EDGE_T)
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	else if (flags & REPEAT_T)
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	else if (flags & MIRRORED_REPEAT_T)
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}

void oglGenerateNew2DTexture(texture *t, v2i size)
{
	SET_FLAG(t->loadFlags, TEX_LOADED);
	t->size = size;
	glGenTextures(1, &t->glHandle);
	glBindTexture(GL_TEXTURE_2D, t->glHandle);

	_SetTexParameters(t->flags);
	
	GLint internalFormat = GL_RGBA;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;
	if (t->storageFlags & STORAGE_R)
	{
		if (t->storageFlags & STORAGE_UNSIGNED_BYTE)
			internalFormat = GL_R8;
		if (t->storageFlags & STORAGE_FLOAT_16)
		{
			internalFormat = GL_R16F;
			type = GL_FLOAT;
		}

		format = GL_RED;
	}
	else if (t->storageFlags & STORAGE_RGB)
	{
		if (t->storageFlags & STORAGE_UNSIGNED_BYTE)
			internalFormat = GL_RGB8;
		if (t->storageFlags & STORAGE_FLOAT_16)
		{
			internalFormat = GL_RGB16F;
			type = GL_FLOAT;
		}

		format = GL_RGB;
	}
	else if (t->storageFlags & STORAGE_RGBA)
	{
		if (t->storageFlags & STORAGE_UNSIGNED_BYTE)
			internalFormat = GL_RGB8;
		if (t->storageFlags & STORAGE_FLOAT_16)
		{
			internalFormat = GL_RGB16F;
			type = GL_FLOAT;
		}

		format = GL_RGBA;
	}
	else if (t->storageFlags & STORAGE_DEPTH_STENCIL)
	{
		internalFormat = GL_DEPTH24_STENCIL8;
		format = GL_DEPTH_STENCIL;
		type = GL_UNSIGNED_INT_24_8;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.w, size.h, 0, format, type, 0);
}

void oglLoadTexture2DIntoGPU(texture *t)
{
	SET_FLAG(t->loadFlags, TEX_LOADED);
	t->lastUsed = 0xffffffff;

	glGenTextures(1, &t->glHandle);
	glBindTexture(GL_TEXTURE_2D, t->glHandle);

	_SetTexParameters(t->flags);

	assert (t->loadFlags & TEX_FROM_FILE);
	int n, w, h;
	uint8_t *image_data = stbi_load(t->fileName, &w, &h, &n, 0);

	t->size = V2I(w, h);
	t->n = n;

	glTexImage2D(GL_TEXTURE_2D,
				 0,								//mipmap level
				 GL_RGBA,						//gl internal format
				 w,
				 h,
				 0,								//border (must be 0)
				 n == 3 ? GL_RGB : GL_RGBA,		//pixel data format
				 GL_UNSIGNED_BYTE,				//pixel data type
				 image_data);

	if (t->loadFlags & TEX_SHOULD_GEN_MIPMAPS)
		oglGenerateMipmaps(t);

	stbi_image_free(image_data);
}

void oglGenerateMipmaps(texture *t)
{
	assert(t->loadFlags & TEX_LOADED);
	assert(t->flags & (MIN_FILTERING_MIPMAP_NEAREST | 
					   MIN_FILTERING_MIPMAP_LINEAR | 
					   MIN_FILTERING_2_MIPMAPS_NEAREST | 
					   MIN_FILTERING_2_MIPMAPS_LINEAR));
	glBindTexture(GL_TEXTURE_2D, t->glHandle);
	glGenerateMipmap(GL_TEXTURE_2D);
	SET_FLAG(t->loadFlags, TEX_MIPMAPS_GENERATED);
}

void oglGenerateFrameBuffer(framebuffer *fb, uint32_t texFlags, v2i size, bool genColor, bool genDepthStencil, uint32_t colorFormat)
{
    glGenFramebuffers(1, &fb->glHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->glHandle);

	memset(&fb->color, '\0', sizeof(texture));
	memset(&fb->depthStencil, '\0', sizeof(texture));

	assert(genColor || genDepthStencil);

	if (genColor)
	{
		fb->color.storageFlags = colorFormat;
		fb->color.flags = texFlags;
		oglGenerateNew2DTexture(&fb->color, size);
		//TODO: change this to allow different types of textures?
		assert(texFlags & TEX_2D);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->color.glHandle, 0);
	}
	if (genDepthStencil)
	{
		fb->depthStencil.storageFlags = STORAGE_DEPTH_STENCIL;
		fb->depthStencil.size = size;
		glGenRenderbuffers(1, &fb->depthStencil.glHandle);
		glBindRenderbuffer(GL_RENDERBUFFER, fb->depthStencil.glHandle);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.w, size.h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb->depthStencil.glHandle);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void oglLoadShader(shader *s, char *vertShaderFile, char *fragShaderFile)
{
	char *vertSrc = ReadFileIntoString(vertShaderFile);
	char *fragSrc = ReadFileIntoString(fragShaderFile);

    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    // Compile Fragment Shader
    glShaderSource(VertexShaderID, 1, &vertSrc, NULL);
    glCompileShader(VertexShaderID);
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
		DBG("Vertex shader problem for: %s", vertShaderFile);
		char *message = (char *)malloc(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, message);
		DBG("%s", message);
		free (message);
    }
    
    // Compile Fragment Shader
    glShaderSource(FragmentShaderID, 1, &fragSrc, NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
		DBG("Fragment shader problem for: %s", fragShaderFile);
		char *message = (char *)malloc(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, message);
		DBG("%s", message);
		free(message);
    }
    
    // Link the program
    s->glHandle = glCreateProgram();
    glAttachShader(s->glHandle, VertexShaderID);
    glAttachShader(s->glHandle, FragmentShaderID);
    glLinkProgram(s->glHandle);
    
    // Check the program
    glGetProgramiv(s->glHandle, GL_LINK_STATUS, &Result);
    glGetProgramiv(s->glHandle, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
		DBG("Problem linking program: %s", vertShaderFile);
		char *message = (char *)malloc(InfoLogLength + 1);
        glGetProgramInfoLog(s->glHandle, InfoLogLength, NULL, message);
		DBG("%s", message);
		free(message);
    }
    
    glDetachShader(s->glHandle, VertexShaderID);
    glDetachShader(s->glHandle, FragmentShaderID);
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
}

void oglCreateVAOWithData(vertex_array_object *vao, vertex_type type, int numVertices, void *data)
{
	vao->type = VAO_DRAW_ARRAYS;
	vao->numVertices = numVertices;

	glGenVertexArrays(1, &vao->vaoHandle);
	glBindVertexArray(vao->vaoHandle);

	glGenBuffers(1, &vao->vbHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vao->vbHandle);

	size_t dataSizeInBytes = 0;
	switch (type)
	{
		case BASIC_VERTEX:
		{
			dataSizeInBytes = sizeof(basic_vertex);
			vao->vertexAttributes = ATTR_POSITION | ATTR_NORMAL | ATTR_UV | ATTR_COLOR;
		} break;

		case SKELETAL_VERTEX:
		{
			dataSizeInBytes = sizeof(skeletal_vertex);
			vao->vertexAttributes = ATTR_POSITION | ATTR_NORMAL | ATTR_UV | ATTR_COLOR | ATTR_JOINT_WEIGHTS | ATTR_JOINT_INDICES;
		} break;
	}

	glBufferData(GL_ARRAY_BUFFER, dataSizeInBytes * numVertices, data, GL_STATIC_DRAW);

	switch (type)
	{
		case BASIC_VERTEX:
		{
			glVertexAttribPointer(LAYOUT_LOC_POSITION, 3, GL_FLOAT, GL_FALSE, dataSizeInBytes, (void *)(offsetof(basic_vertex, position)));
			glVertexAttribPointer(LAYOUT_LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, dataSizeInBytes, (void *)(offsetof(basic_vertex, normal)));
			glVertexAttribPointer(LAYOUT_LOC_UV, 2, GL_FLOAT, GL_FALSE, dataSizeInBytes, (void *)(offsetof(basic_vertex, uv)));
			glVertexAttribPointer(LAYOUT_LOC_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, dataSizeInBytes, (void *)(offsetof(basic_vertex, color)));

			glEnableVertexAttribArray(LAYOUT_LOC_POSITION);
			glEnableVertexAttribArray(LAYOUT_LOC_NORMAL);
			glEnableVertexAttribArray(LAYOUT_LOC_UV);
			glEnableVertexAttribArray(LAYOUT_LOC_COLOR);
		} break;

		case SKELETAL_VERTEX:
		{
			glVertexAttribPointer(LAYOUT_LOC_POSITION, 3, GL_FLOAT, GL_FALSE, dataSizeInBytes, (void *)(offsetof(skeletal_vertex, position)));
			glVertexAttribPointer(LAYOUT_LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, dataSizeInBytes, (void *)(offsetof(skeletal_vertex, normal)));
			glVertexAttribPointer(LAYOUT_LOC_UV, 2, GL_FLOAT, GL_FALSE, dataSizeInBytes, (void *)(offsetof(skeletal_vertex, uv)));
			glVertexAttribPointer(LAYOUT_LOC_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, dataSizeInBytes, (void *)(offsetof(skeletal_vertex, color)));
			glVertexAttribPointer(LAYOUT_LOC_JOINT_WEIGHTS, 3, GL_FLOAT, GL_FALSE, dataSizeInBytes, (void *)(offsetof(skeletal_vertex, weights)));
			glVertexAttribIPointer(LAYOUT_LOC_JOINT_INDICES, 3, GL_INT, dataSizeInBytes, (void *)(offsetof(skeletal_vertex, jointIndices)));

			glEnableVertexAttribArray(LAYOUT_LOC_POSITION);
			glEnableVertexAttribArray(LAYOUT_LOC_NORMAL);
			glEnableVertexAttribArray(LAYOUT_LOC_UV);
			glEnableVertexAttribArray(LAYOUT_LOC_COLOR);
			glEnableVertexAttribArray(LAYOUT_LOC_JOINT_WEIGHTS);
			glEnableVertexAttribArray(LAYOUT_LOC_JOINT_INDICES);
		} break;
	}

	glBindVertexArray(0);
}

void oglCreateVAOWithData(vertex_array_object *vao, vertex_type type, int numVertices, void *data, int numIndices, void *indices)
{
	oglCreateVAOWithData(vao, type, numVertices, data);
	vao->type = VAO_DRAW_ELEMENTS;
	vao->numIndices = numIndices;

	glBindVertexArray(vao->vaoHandle);

	glGenBuffers(1, &vao->ebHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->ebHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t)*numIndices, indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
}
