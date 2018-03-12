#version 330 core

uniform sampler2D Texture;
uniform vec2 Resolution;

in vec2 ThruUV;

out vec4 color;

const vec4 outlineColor = vec4(0,1,0,1);
const vec4 noColor = vec4(0,0,0,0);

void main()
{
	float current = texture(Texture, ThruUV).r;
	if (current > 0.0)
	{
		color = noColor;
	}
	else
	{
		float total = 0;
		float oneOverResX = 1.0 / Resolution.x;
		float twoOverResX = 2.0 / Resolution.x;
		float oneOverResY = 1.0 / Resolution.y;
		float twoOverResY = 2.0 / Resolution.y;
		total += texture(Texture, ThruUV + vec2(-oneOverResX, -twoOverResY)).r;
		total += texture(Texture, ThruUV + vec2(0, -twoOverResY)).r;
		total += texture(Texture, ThruUV + vec2(oneOverResX, -twoOverResY)).r;

		total += texture(Texture, ThruUV + vec2(-twoOverResX, -oneOverResY)).r;
		total += texture(Texture, ThruUV + vec2(-oneOverResX, -oneOverResY)).r;
		total += texture(Texture, ThruUV + vec2(0, -oneOverResY)).r;
		total += texture(Texture, ThruUV + vec2(oneOverResX, -oneOverResY)).r;
		total += texture(Texture, ThruUV + vec2(twoOverResX, -oneOverResY)).r;

		total += texture(Texture, ThruUV + vec2(-twoOverResX, 0)).r;
		total += texture(Texture, ThruUV + vec2(-oneOverResX, 0)).r;
		total += texture(Texture, ThruUV + vec2(oneOverResX, 0)).r;
		total += texture(Texture, ThruUV + vec2(twoOverResX, 0)).r;

		total += texture(Texture, ThruUV + vec2(-twoOverResX, oneOverResY)).r;
		total += texture(Texture, ThruUV + vec2(-oneOverResX, oneOverResY)).r;
		total += texture(Texture, ThruUV + vec2(0, oneOverResY)).r;
		total += texture(Texture, ThruUV + vec2(oneOverResX, oneOverResY)).r;
		total += texture(Texture, ThruUV + vec2(twoOverResX, oneOverResY)).r;

		total += texture(Texture, ThruUV + vec2(-oneOverResX, twoOverResY)).r;
		total += texture(Texture, ThruUV + vec2(0, twoOverResY)).r;
		total += texture(Texture, ThruUV + vec2(oneOverResX, twoOverResY)).r;

		if (total > 0.1)
		{
			total *= 0.3;
			color = total * outlineColor;
		}
		else
		{
			color = noColor;
		}
	}
} 
