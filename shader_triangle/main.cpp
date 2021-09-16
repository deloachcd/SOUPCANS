#include <stdio.h>
#include <math.h>

#include <memory>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/souputils/glDebug.hpp"
#include "../include/souputils/glHelpers.hpp"
#include "../include/souputils/glfwHelpers.hpp"
#include "../include/souputils/convenience.hpp"

#define SOUP_GL_DEBUG_CONTEXT

using namespace souputils::gldebug;
using namespace souputils::glhelpers;
using namespace souputils::glfwhelpers;
using namespace souputils::convenience;

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "FATAL: could not initialize GLFW3!\n");
        return 1;
    }

#ifdef SOUP_GL_DEBUG_CONTEXT
	glfwSetWindowHintProfile(SOUP_GLFW_DEBUG_PROFILE);
#else
	glfwSetWindowHintProfile(SOUP_GLFW_RELEASE_PROFILE);
#endif

    std::unique_ptr<glfwDisplayObjects> display_objects = glfwGetDisplayObjects();
	int win_width = 1600, win_height = 1200;
    GLFWwindow* window = glfwCreateWindow(
        win_width, win_height,
        "OpenGL program that hasn't rendered anything yet",
        NULL, NULL
    );
    if (!window) {
        fprintf(stderr, "FATAL: could not open window with GLFW3!\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (gl3wInit()) {
		fprintf(stderr, "OH NO INDEPENDENCE DAY (gl3wInit failed)\n");
		return 1;
    }

#ifdef SOUP_GL_DEBUG_CONTEXT
	enableSoupDebugContext();
	glLogInfo("Renderer: %s\n", glGetString(GL_RENDERER));
	glLogInfo("OpenGL version supported: %s\n", glGetString(GL_VERSION));
#endif

	glfwSwapInterval(1);  // enable vsync
    glfwSetFramebufferSizeCallback(window, updateGlViewportOnWindowResize);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

	// the main logic
    glm::vec3 triangle_vectors[] = {
        glm::vec3( 0.0f,  0.5f,  0.0f),
        glm::vec3( 0.5f, -0.5f,  0.0f),
        glm::vec3(-0.5f, -0.5f,  0.0f)
    };

	std::unique_ptr<float[]> points = flatten(triangle_vectors, sizeof(triangle_vectors));

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vectors),
				 points.get(), GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(0);

    std::unique_ptr<shaderSrc> vertex_src, fragment_src;
    vertex_src = loadShaderFile(
        GL_VERTEX_SHADER, "res/shaders/vertex.glsl"
    );
    fragment_src = loadShaderFile(
        GL_FRAGMENT_SHADER, "res/shaders/fragment.glsl"
    );
    GLuint vs = compileShaderSrc(vertex_src.get());
    GLuint fs = compileShaderSrc(fragment_src.get());

    GLuint shader_prog = glCreateProgram();
    glAttachShader(shader_prog, vs);
    glAttachShader(shader_prog, fs);
    glLinkProgramSafe(shader_prog);

    int matrix_location = glGetUniformLocation(shader_prog, "matrix");
    glUseProgram(shader_prog);

	float scale = 1.0f;
	//float hcorr = (static_cast<float>(win_height) / static_cast<float>(win_width));
    glm::mat4 widescreen_matrix{
		scale,         0.0f,  0.0f, 0.0f,
		 0.0f,        scale,  0.0f, 0.0f,
		 0.0f,         0.0f, scale, 0.0f,
		 0.0f,         0.0f,  0.0f, 1.0f,
    };

    glUniformMatrix4fv(matrix_location, 1, GL_FALSE,
					   glm::value_ptr(widescreen_matrix));

	std::unique_ptr<fpsCounter> fcounter(new fpsCounter);

	int cvector_location = glGetUniformLocation(shader_prog, "cvector");
	glm::vec3 color_iterator;
	const int N_COLOR_SHIFT_FRAMES = 25;
	const float DELTA = 1.0f / static_cast<float>(N_COLOR_SHIFT_FRAMES);
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;

	enum FRAME_COLOR {RED, GREEN, BLUE};
	enum FRAME_OPERATION {INC, DEC};
	FRAME_COLOR fcolor = RED;
	FRAME_OPERATION r_op = DEC;
	FRAME_OPERATION g_op = DEC;
	FRAME_OPERATION b_op = DEC;

    while (!glfwWindowShouldClose(window)) {
		switch (fcolor) {
		case RED:
			if (r < 0.0f || r > 1.0f) {
				r_op = (r_op == INC) ? DEC : INC;
			}
			r = (r_op == INC) ? r + DELTA : r - DELTA;
			fcolor = GREEN;
			break;
		case GREEN:
			if (g < 0.0f || g > 1.0f) {
				g_op = (g_op == INC) ? DEC : INC;
			}
			g = (g_op == INC) ? g + DELTA : g - DELTA;
			fcolor = BLUE;
			break;
		case BLUE:
			if (b < 0.0f || b > 1.0f) {
				b_op = (b_op == INC) ? DEC : INC;
			}
			b = (b_op == INC) ? b + DELTA : b - DELTA;
			fcolor = RED;
			break;
		}
		//printf("%f %f %f\n", r, g, b);

		color_iterator = glm::vec3{r, g, b};
		glUniform3fv(cvector_location, 1,
						glm::value_ptr(color_iterator));
		updateFPSCounter(window, fcounter.get());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader_prog);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwPollEvents();
		glfwSwapBuffers(window);

		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
			return 0;
		}
    }
}
