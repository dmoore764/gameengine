#version 330 core

in vec4 ThruColor;

out vec4 color;

void main() {
	if (int(gl_FragCoord.x) % 2 == 0 && int(gl_FragCoord.y) % 2 == 0)
	{
		color = ThruColor;
	}
	else
	{
		discard;
	}
}
