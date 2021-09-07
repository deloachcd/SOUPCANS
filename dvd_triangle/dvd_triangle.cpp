#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>

#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/glDebug.hpp"
#include "../include/glHelpers.hpp"

using glhelpers::displayObjects;
using glhelpers::shaderSrc;

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    std::unique_ptr<displayObjects> display_objects = glhelpers::getDisplayObjects();
    GLFWwindow* window = glhelpers::glfwCreatePrimaryWindow(
        display_objects->vidmode->width/2, display_objects->vidmode->width/3,
        "OpenGL program that hasn't rendered anything yet",
        NULL, NULL
    );
    if (!window) {
        GL_LOG_ERROR() << "ERROR: could not open window with GLFW3";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (gl3wInit()) {
       GL_LOG_ERROR()  << "OH NO INDEPENDENCE DAY (gl3wInit failed)";
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gldebug::glDebugCallback, nullptr);
    glfwSetErrorCallback(gldebug::glfwErrorCallback);
    glfwSetWindowSizeCallback(window, 
            glhelpers::glfw_primary_window_size_callback);
    glfwSetFramebufferSizeCallback(window, 
            glhelpers::glfw_default_framebuffer_size_callback);
    GL_LOG_RESET();
    /* gldebug::logGLParams(); */

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    GL_LOG_INFO() << "Renderer: " << renderer;
    GL_LOG_INFO() << "OpenGL version supported: " << version;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Seed random values and generate for x and y positions
    srand(time(NULL));
    float X_POS = (float)(rand() % 100) / 200;
    float Y_POS = (float)(rand() % 100) / 200;

    glm::vec3 vectors[] = {
        glm::vec3( 0.0f,  0.5f,  0.0f),
        glm::vec3( 0.5f, -0.5f,  0.0f),
        glm::vec3(-0.5f, -0.5f,  0.0f)
    };

    GLfloat colors[] = {
         1.0f,  0.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  0.0f,  1.0f,
    };

    GLfloat cmatrix[] = {
         1.0f,  0.0f, 0.0f,
         0.0f,  1.0f, 0.0f,
         0.0f,  0.0f, 1.0f,
         0.0f,  0.0f, 0.0f,
    };

    glm::mat4 matrix{
         0.5f,  0.0f, 0.0f, 0.0f,
         0.0f,  0.5f, 0.0f, 0.0f,
         0.0f,  0.0f, 0.5f, 0.0f,
        X_POS, Y_POS, 0.0f, 1.0f,
    };

    // VBOs
    GLuint points_vbo, colors_vbo;
    glhelpers::ufloat_ptr points = glhelpers::flatten(vectors, sizeof(vectors));

    glGenBuffers(1, &points_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vectors), points.get(), GL_STATIC_DRAW);
    glGenBuffers(1, &colors_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    // VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    /* Shader program initialization logic */
    std::unique_ptr<shaderSrc> vertex_src, fragment_src;
    vertex_src = glhelpers::load_shader_file(
        GL_VERTEX_SHADER, "res/shaders/vertex.glsl"
    );
    fragment_src = glhelpers::load_shader_file(
        GL_FRAGMENT_SHADER, "res/shaders/fragment.glsl"
    );
    GLuint vs = glhelpers::compile_shaderSrc(vertex_src.get());
    GLuint fs = glhelpers::compile_shaderSrc(fragment_src.get());

    // Combine compiled shaders into GPU shader program and link it
    GLuint shader_prog = glCreateProgram();
    glAttachShader(shader_prog, vs);
    glAttachShader(shader_prog, fs);
    glhelpers::gl_link_program(shader_prog);

    // Send matrix to shader
    int matrix_location = glGetUniformLocation(shader_prog, "matrix");
    int cmatrix_location = glGetUniformLocation(shader_prog, "cmatrix");
    glUseProgram(shader_prog);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(matrix));
    glUniformMatrix3fv(cmatrix_location, 1, GL_FALSE, cmatrix);

    // Render loop
    static double previous_seconds = glfwGetTime();
    double current_seconds = glfwGetTime();
    double elapsed_seconds = current_seconds - previous_seconds;

    GLfloat speed_x = 0.75f;
    GLfloat speed_y = 0.75f;
    GLfloat SPEED_LIMIT = 1.25;
    GLfloat last_position_x = X_POS;
    GLfloat last_position_y = Y_POS;
    glhelpers::SimpleTimer timer = glhelpers::SimpleTimer();
    while (!glfwWindowShouldClose(window)) {
        glhelpers::update_fps_counter(window);

        // timer for doing animation
        timer.update();

        // reverse direction when going too far left, right, up or down
        if (fabs(last_position_x) > 0.75f || fabs(last_position_y) > 0.75f) {
            // Randomize matrix and transform colors with it
            for (int i = 0; i < sizeof(cmatrix) / sizeof(GLfloat); i++) {
                cmatrix[i] = (GLfloat)(rand() % 100) / 100;
            }

            if (fabs(last_position_x) > 0.75f) {
                // x direction gets to speed up a little bit to prevent "loops"
                if (speed_x >= SPEED_LIMIT) {
                    speed_x = (speed_x < -1) ? 0.75f : -0.75f;
                } else {
                    speed_x = -(speed_x + 0.2f);
                }
                last_position_x += (timer.getElapsedSeconds() * speed_x);
            }
            if (fabs(last_position_y) > 0.75f) {
                speed_y = -speed_y;
                last_position_y += (timer.getElapsedSeconds() * speed_y);
            }
        }

        // update matrix
        matrix[3][0] = (timer.getElapsedSeconds() * speed_x) + last_position_x;
        last_position_x = matrix[3][0];
        matrix[3][1] = (timer.getElapsedSeconds() * speed_y) + last_position_y;
        last_position_y = matrix[3][1];
        glUseProgram(shader_prog);
        glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(matrix));
        glUniformMatrix3fv(cmatrix_location, 1, GL_FALSE, cmatrix);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 
                glhelpers::get_glfw_primary_window_width(), 
                glhelpers::get_glfw_primary_window_height());
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwPollEvents();
        glfwSwapBuffers(window);

        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, 1);
        }
    }

    glfwTerminate();

    return 0;
}
