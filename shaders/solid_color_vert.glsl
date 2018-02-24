#version 330 core
layout (location = 0) in vec3 Pos;
layout (location = 3) in vec4 Color;

uniform mat4 PerspView;

out vec4 ThruColor;

void main()
{
	ThruColor = Color / 255.0;
    gl_Position = PerspView * vec4(Pos.x, Pos.y, Pos.z, 1.0);
}
