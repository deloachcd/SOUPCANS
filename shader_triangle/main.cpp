#include <stdio.h>
#include <math.h>

#include <memory>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include "../include/souputils/glDebug.hpp"
#include "../include/souputils/glHelpers.hpp"
#include "../include/souputils/glfwHelpers.hpp"
#include "../include/souputils/convenience.hpp"

//#include "../include/stb/stb_image.hpp"

#define SOUP_GL_DEBUG_CONTEXT

#ifdef SOUP_GL_DEBUG_CONTEXT
/* TODO
#ifdef __unix__
#include <sys/inotify.h>
#endif
*/
#endif

using namespace souputils::gldebug;
using namespace souputils::glhelpers;
using namespace souputils::glfwhelpers;
using namespace souputils::convenience;

GLuint compileSimpleShaderProgram(const char* vertex_shader_fname,
								  const char* fragment_shader_fname) {
	/* I can generalize this logic later if I need to link more
	   than a single vertex/fragment shader
	*/
    std::unique_ptr<shaderSrc> vertex_src, fragment_src;
    vertex_src = loadShaderFile(
        GL_VERTEX_SHADER, vertex_shader_fname
    );
    fragment_src = loadShaderFile(
        GL_FRAGMENT_SHADER, fragment_shader_fname
    );
    GLuint vs = compileShaderSrc(vertex_src.get());
    GLuint fs = compileShaderSrc(fragment_src.get());

    GLuint new_shader_prog = glCreateProgram();
    glAttachShader(new_shader_prog, vs);
    glAttachShader(new_shader_prog, fs);
    glLinkProgramSafe(new_shader_prog);

	return new_shader_prog;
}

void reloadShaderProgramFromFiles(GLuint* program,
								  const char* vertex_shader_fname,
								  const char* fragment_shader_fname) {
    //std::unique_ptr<shaderSrc> vertex_src, fragment_src;
	GLuint old_program;
	GLuint new_program = compileSimpleShaderProgram(
		vertex_shader_fname,
		fragment_shader_fname
		);
	if (new_program) {
		old_program = *program;
		*program = new_program;
		glDeleteProgram(old_program);
	}
}

template <class T>
inline GLuint vboFromFlattenedArray(T* arr, size_t size_arr) {
	GLsizeiptr size_arr_cast = static_cast<GLsizeiptr>(size_arr);

	GLint current_array_buffer; // = glGetIntegerv(GL_ARRAY_BUFFER);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &current_array_buffer);

	std::unique_ptr<float[]> arr_flatten = flatten(arr, size_arr);

	GLuint new_vbo;
	glGenBuffers(1, &new_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, new_vbo);
	glBufferData(GL_ARRAY_BUFFER, size_arr_cast, arr_flatten.get(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(current_array_buffer));
	return new_vbo;
}

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
    int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
    unsigned char* texture_img_data = stbi_load("res/img/cloud_texture_crop.jpg",
												&width, &height, &nrChannels, 0);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    if (texture_img_data) {
		// NOTE: this will just segfault if image dimensions aren't to spec!
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
					 0, GL_RGB, GL_UNSIGNED_BYTE, texture_img_data);
        //glGenerateMipmap(GL_TEXTURE_2D);
		glLogInfo("%d %d\n", width, height);
    } else {
        printf("Failed to load image!\n");
    }
    stbi_image_free(texture_img_data);

	glm::vec2 skybox_positions[] = {
		glm::vec2( 0.5f,  0.5f),  // top right
		glm::vec2( 0.5f, -0.5f),  // bottom right
		glm::vec2(-0.5f, -0.5f),  // bottom left
		glm::vec2(-0.5f,  0.5f)   // top left 
	};
	GLuint skybox_vbo = vboFromFlattenedArray<glm::vec2>(skybox_positions,
														 sizeof(skybox_positions));

    glm::vec2 triangle_positions[] = {
        glm::vec2( 0.0f,  0.5f),
        glm::vec2( 0.5f, -0.5f),
        glm::vec2(-0.5f, -0.5f)
    };
	GLuint triangle_vbo = vboFromFlattenedArray<glm::vec2>(triangle_positions,
														   sizeof(triangle_positions));

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// triangle @ location = 0
	glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

	// triangle @ location = 0
	glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

	const char* vertf = "res/shaders/vertex.glsl";
	const char* fragf = "res/shaders/fragment.glsl";
	GLuint shader_prog = compileSimpleShaderProgram(vertf, fragf);

/*
  TODO: implement live reloading with inotify
#ifdef SOUP_GL_DEBUG_CONTEXT
#ifdef __unix__
	int vertf_channel = inotify_init();
	int fragf_channel = inotify_init();
	if (vertf_channel < 0 || fragf_channel < 0) {
		perror("inotify_init");
	}
	int vertf_watcher = inotify_add_watch(vertf_channel, vertf,
										  IN_MODIFY | IN_DELETE);
	int fragf_watcher = inotify_add_watch(fragf_channel, fragf,
										  IN_MODIFY | IN_DELETE);
#endif
#endif
*/

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

	const int N_COLOR_SHIFT_FRAMES = 7500;
	const float DELTA = 1.0f / static_cast<float>(N_COLOR_SHIFT_FRAMES);

	enum FRAME_OPERATION {INC, DEC};
	int intensity_location = glGetUniformLocation(shader_prog, "intensity");
	FRAME_OPERATION i_op = INC;
	float intensity = 0.0f;
    while (!glfwWindowShouldClose(window)) {
		if (intensity < 0.0f || intensity > 1.0f) {
			i_op = (i_op == INC) ? DEC : INC;
		}
		intensity = (i_op == INC) ? intensity + DELTA : intensity - DELTA;

		updateFPSCounter(window, fcounter.get());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader_prog);
		glUniform1f(intensity_location, intensity);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwPollEvents();
		glfwSwapBuffers(window);

		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_R)) {
			reloadShaderProgramFromFiles(&shader_prog, vertf, fragf);
			glUseProgram(shader_prog);
			intensity_location = glGetUniformLocation(shader_prog, "intensity");
			glUniform1f(intensity_location, intensity);
			matrix_location = glGetUniformLocation(shader_prog, "matrix");
			glUniformMatrix4fv(matrix_location, 1, GL_FALSE,
							   glm::value_ptr(widescreen_matrix));
		}
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
			return 0;
		}
    }
}
