#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 3) in vec4 vertexColor;

out vec4 ThruColor;

uniform mat4 PerspView;
uniform mat4 ModelMat;

void main() {
	ThruColor = vertexColor / 255.0;
	gl_Position = PerspView * ModelMat * vec4(vertexPosition, 1.0);
}
