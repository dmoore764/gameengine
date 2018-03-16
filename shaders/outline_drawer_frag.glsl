#version 330 core

uniform sampler2D Texture;
uniform vec2 Resolution;

in vec2 ThruUV;

out vec4 color;

const vec4 noColor = vec4(0,0,0,0);

void main()
{
	vec2 current = texture(Texture, ThruUV).rg;
	if (current.r + current.g> 0.0)
	{
		color = noColor;
	}
	else
	{
		vec2 outColor = vec2(0.0,0.0);
		float oneOverResX = 1.0 / Resolution.x;
		float twoOverResX = 2.0 / Resolution.x;
		float oneOverResY = 1.0 / Resolution.y;
		float twoOverResY = 2.0 / Resolution.y;
		outColor += texture(Texture, ThruUV + vec2(-oneOverResX, -twoOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(0, -twoOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(oneOverResX, -twoOverResY)).rg;

		outColor += texture(Texture, ThruUV + vec2(-twoOverResX, -oneOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(-oneOverResX, -oneOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(0, -oneOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(oneOverResX, -oneOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(twoOverResX, -oneOverResY)).rg;

		outColor += texture(Texture, ThruUV + vec2(-twoOverResX, 0)).rg;
		outColor += texture(Texture, ThruUV + vec2(-oneOverResX, 0)).rg;
		outColor += texture(Texture, ThruUV + vec2(oneOverResX, 0)).rg;
		outColor += texture(Texture, ThruUV + vec2(twoOverResX, 0)).rg;

		outColor += texture(Texture, ThruUV + vec2(-twoOverResX, oneOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(-oneOverResX, oneOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(0, oneOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(oneOverResX, oneOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(twoOverResX, oneOverResY)).rg;

		outColor += texture(Texture, ThruUV + vec2(-oneOverResX, twoOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(0, twoOverResY)).rg;
		outColor += texture(Texture, ThruUV + vec2(oneOverResX, twoOverResY)).rg;

		if (outColor.r > 0.1)
		{
			color = vec4(0.5, 1.0, 0.8, 1.0);
		}
		else if (outColor.g > 0.1)
		{
			color = vec4(1.0, 1.0, 1.0, 1.0);
		}
		else
		{
			color = noColor;
		}
	}
} 
