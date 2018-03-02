#version 330 core
layout (location = 0) in vec3 Pos;
layout (location = 2) in vec2 UV;
layout (location = 3) in vec4 Color;

uniform mat4 PerspView;

out vec4 ThruColor;
out vec2 ThruUV;

void main()
{
	ThruColor = Color / 255.0;
	ThruUV = UV;
    gl_Position = PerspView * vec4(Pos.x, Pos.y, Pos.z, 1.0);
}
