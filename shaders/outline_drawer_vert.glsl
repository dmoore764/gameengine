#version 330 core
layout (location = 0) in vec3 Pos;
layout (location = 2) in vec2 UV;

uniform mat4 PerspView;

out vec2 ThruUV;

void main()
{
	ThruUV = UV;
    gl_Position = PerspView * vec4(Pos.x, Pos.y, Pos.z, 1.0);
}
