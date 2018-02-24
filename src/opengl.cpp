#include "opengl.h"

#include "file_utilities.h"
#include "compiler_helper.h"
#include "stdlib.h"
#include "stddef.h"

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
}

