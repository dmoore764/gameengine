#version 330 core
layout(location = 0) in vec3 vertexPosition;

uniform mat4 PerspView;
uniform mat4 ModelMat;

void main() {
	gl_Position = PerspView * ModelMat * vec4(vertexPosition, 1.0);
}
