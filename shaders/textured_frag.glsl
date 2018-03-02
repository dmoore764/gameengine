#version 330 core

uniform sampler2D Texture;

in vec4 ThruColor;
in vec2 ThruUV;

out vec4 color;

void main()
{
    color = ThruColor * texture(Texture, ThruUV);
} 
