#include "stdio.h"
#include "window.h"
#include "opengl.h"
#include "compiler_helper.h"
#include "game_math.h"

int main (int argCount, char **args)
{
	window w;
	WinCreate(&w, V2I(640, 480));

	quat q0 = CreateQuat(90, V3(0,0,1));
	m4 q2m = QuatToM4(q0);
	quat m2q = M4ToQuat(q2m);
	v3 p = V3(1,0,0);
	p = RotateByQuat(p, m2q);

	m4 identity = M4();
	m4 tx = MakeTranslation(10, 11, 12);
	m4 la = MakeLookat(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
	m4 ps = MakePerspective(70.0f, (float)w.size.w/(float)w.size.h, 0.5f, 120.0f);

	m4 pv = ps*la;

	shader s;
	oglLoadShader(&s, "../shaders/solid_color_vert.glsl", "../shaders/solid_color_frag.glsl");
	s.vertexAttributes = ATTR_POSITION | ATTR_COLOR;

	basic_vertex vertData[3] = {
		{{-3,30,0}, {0,0,0}, {0,0}, MAKE_COLOR(255,255,255,255)},
		{{3,30,0}, {0,0,0}, {0,0}, MAKE_COLOR(0,255,255,255)},
		{{0,30,6}, {0,0,0}, {0,0}, MAKE_COLOR(255,0,255,255)},
	};

	vertex_array_object vao;

	oglCreateVAOWithData(&vao, BASIC_VERTEX, 3, vertData);

	bool running = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		WinClear(0,0,0);
		glUseProgram(s.glHandle);
		GLint perspView = glGetUniformLocation(s.glHandle, "PerspView");
		glUniformMatrix4fv(perspView, 1, false, &pv[0]);

		glBindVertexArray(vao.vaoHandle);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		WinSwapBuffers(&w);

	}

	WinClose(&w);
	return 0;
}
