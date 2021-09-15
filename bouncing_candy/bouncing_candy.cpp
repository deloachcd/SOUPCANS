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
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../include/glDebug.hpp"
#include "../include/glHelpers.hpp"

void nanoDelay(unsigned int nanoseconds) {
	timespec frame_delay = { 0,          /* seconds */
							 nanoseconds /* nanoseconds */ };
	timespec remaining = { 0, 0 };

	nanosleep(&frame_delay, &remaining);
}

// structs defined in glhelpers.hpp for convient grouping of things
using glhelpers::displayObjects;
using glhelpers::shaderSrc;

class MovingObject {
    private:
        float velocity_x;
        float velocity_y; float prev_pos_x; 
        float prev_pos_y;
        float* model_pos_x;
        float* model_pos_y;

        void updateModel(float pos_x, float pos_y) {
            *(this->model_pos_x) = pos_x;
            *(this->model_pos_y) = pos_y;
        }

    public:
        MovingObject(float vel_x, float vel_y, float* model_x, float* model_y) {
            this->velocity_x = vel_x;
            this->velocity_y = vel_y;
            this->model_pos_x = model_x;
            this->model_pos_y = model_y;
            this->prev_pos_x = *model_x;
            this->prev_pos_y = *model_y;
        }

        void recordPosition() {
            this->prev_pos_x = *(this->model_pos_x);
            this->prev_pos_y = *(this->model_pos_y);
        }

        void applyVelocity(double elapsed_seconds) {
            this->updateModel(
                this->prev_pos_x + (elapsed_seconds * this->velocity_x),
                this->prev_pos_y + (elapsed_seconds * this->velocity_y)
            );
        }

        void setVelocity(float vel_x, float vel_y) {
            this->velocity_x = vel_x;
            this->velocity_y = vel_y;
        }

        float getVelocityX() {
            return this->velocity_x;
        }

        float getVelocityY() {
            return this->velocity_y;
        }

        float getPositionX() {
            return *(this->model_pos_x);
        }

        float getPositionY() {
            return *(this->model_pos_x);
        }
};

void squish_matrix(glm::mat4& matrix, float obj_y_pos, float obj_radius, float ground_y) {
    float compress_factor = 0.75f;
    float expand_factor = 0.75f;
    float ground_proximity;
    if (obj_y_pos <= ground_y + obj_radius) {
        ground_proximity = fabs(obj_y_pos - obj_radius - ground_y);
        matrix[0][0] = 1.0f + (expand_factor * ground_proximity );
        matrix[1][1] = 1.0f - (compress_factor * ground_proximity);
        matrix[2][2] = 1.0f + (expand_factor * ground_proximity);
    } else {
        matrix[0][0] = 1.0f;
        matrix[1][1] = 1.0f;
        matrix[2][2] = 1.0f;
    }
}

int main() {
    if (!glfwInit()) {
        GL_LOG_ERROR() << "ERROR: could not start GLFW3";
        return 1;
    }

    /* Window hints */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    /* Video mode, window, display */
    std::unique_ptr<displayObjects> display_objects = glhelpers::getDisplayObjects();
    GLFWwindow* window = glfwCreateWindow(
        display_objects->vidmode->width/2, display_objects->vidmode->height/2,
        "OpenGL program that hasn't rendered anything yet",
        NULL, NULL
    );
    if (!window) {
        GL_LOG_ERROR() << "ERROR: could not open window with GLFW3";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
	// disable vsync to test our 60fps cap
	glfwSwapInterval(1);

    /* Initialize extension wrangler library */
    if (gl3wInit()) {
       GL_LOG_ERROR()  << "OH NO INDEPENDENCE DAY (gl3wInit failed)";
    }

    /* Debugging */
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gldebug::glDebugCallback, nullptr);
    glfwSetErrorCallback(gldebug::glfwErrorCallback);
    //gldebug::logGLParams(); // uncomment to log graphics hardware limits

    /* Initialize a new log and populate with version info */
    GL_LOG_RESET();
    GL_LOG_INFO() << "Renderer: " << glGetString(GL_RENDERER);
    GL_LOG_INFO() << "OpenGL version supported: " << glGetString(GL_VERSION);

    /* Callbacks for non-debugging functions */
    glfwSetWindowSizeCallback(window, glhelpers::glfw_primary_window_size_callback);
    glfwSetFramebufferSizeCallback(window, glhelpers::glfw_default_framebuffer_size_callback);

    /* Misc. setup calls to OpenGL's API */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* Matrices and 3d object initialization */
    float scale = 0.3f;
    float wcorr = 0.5625f; // correction factor for widescreen
    glm::mat4 model{
         scale,         0.0f,   0.0f, 0.0f,
          0.0f,  scale/wcorr,   0.0f, 0.0f,
          0.0f,         0.0f,  scale, 0.0f,
          0.0f,         0.0f,   0.0f, 1.0f
    };
    glm::mat4 squish{
          1.0f,  0.0f,  0.0f, 0.0f,
          0.0f,  1.0f,  0.0f, 0.0f,
          0.0f,  0.0f,  1.0f, 0.0f,
          0.0f,  0.0f,  0.0f, 1.0f
    };
    //float* obj_x = &model[3][0];
    float* obj_y = &model[3][1];

    float color_vectors[] = {
        0.22f, 0.00f, 0.23f,
        0.00f, 0.44f, 0.00f,
        0.01f, 0.00f, 0.58f,
        1.00f, 0.11f, 0.00f,
        0.26f, 1.00f, 0.59f,
        0.00f, 0.00f, 1.00f,
        0.55f, 0.00f, 0.56f,
        0.00f, 0.64f, 0.00f,
        0.98f, 0.00f, 0.58f,
        1.00f, 0.66f, 0.00f,
        0.74f, 1.00f, 0.69f,
        0.00f, 0.37f, 1.00f,
        0.84f, 0.00f, 0.10f,
        0.00f, 0.73f, 0.00f,
        0.23f, 0.00f, 0.83f,
    };
    /* float color_vectors[] = { */
    /*      1.0f,  0.0f,  0.0f, */
    /*      0.0f,  1.0f,  0.0f, */
    /*      0.0f,  0.0f,  1.0f, */
    /*      1.0f,  0.0f,  0.0f, */
    /*      0.0f,  1.0f,  0.0f, */
    /*      0.0f,  0.0f,  1.0f, */
    /*      1.0f,  0.0f,  0.0f, */
    /*      0.0f,  1.0f,  0.0f, */
    /*      0.0f,  0.0f,  1.0f, */
    /*      1.0f,  0.0f,  0.0f, */
    /*      0.0f,  1.0f,  0.0f, */
    /*      0.0f,  0.0f,  1.0f, */
    /*      1.0f,  0.0f,  0.0f, */
    /*      0.0f,  1.0f,  0.0f, */
    /*      0.0f,  0.0f,  1.0f, */
    /* }; */

    /* srand(time(NULL)); */
    /* for (int i = 0; i < sizeof(color_vectors) / sizeof(GLfloat); i+=2) { */
    /*     color_vectors[i] = (GLfloat)(rand() % 100) / 100; */
    /*     std::cout << i << " " << color_vectors[i] << std::endl; */
    /* } */

    float p = 0.25; // protrusion factor for pyramid face
    float bucephalus_vectors[] = {
        // front face of inner cube
        -0.5f,  0.5f,  0.5f,          // 0
        -0.5f, -0.5f,  0.5f,          // 1
         0.5f,  0.5f,  0.5f,          // 2
         0.5f, -0.5f,  0.5f,          // 3
         // rear face of inner cube
        -0.5f,  0.5f, -0.5f,          // 4
        -0.5f, -0.5f, -0.5f,          // 5
         0.5f,  0.5f, -0.5f,          // 6
         0.5f, -0.5f, -0.5f,          // 7
         // "pyramid face" vectors
         0.5f+p,  0.0f,    0.0f,      // 8
        -0.5f-p,  0.0f,    0.0f,      // 9
         0.0f,    0.5f+p,  0.0f,      // 10
         0.0f,   -0.5f-p,  0.0f,      // 11
         0.0f,    0.0f,    0.5f+p,    // 12
         0.0f,    0.0f,   -0.5f-p,    // 13
    };
    GLuint bucephalus_indices[] = {
        // front face
        0, 1, 12,
        0, 2, 12,
        3, 1, 12,
        3, 2, 12,
        // rear face
        4, 5, 13,
        4, 6, 13,
        7, 5, 13,
        7, 6, 13,
        // right face
        2, 3, 8, 
        2, 6, 8,
        7, 3, 8,
        7, 6, 8,
        // left face
        0, 1, 9, 
        0, 4, 9,
        5, 1, 9,
        5, 4, 9,
        // top face
        0, 4, 10, 
        0, 2, 10,
        6, 4, 10,
        6, 2, 10,
        // bottom face
        1, 5, 11, 
        1, 3, 11,
        7, 5, 11,
        7, 3, 11,
    };

    /* Binding for our 3d objects */
    GLuint vertex_arr;
    glGenVertexArrays(1, &vertex_arr);
    glBindVertexArray(vertex_arr);

    GLuint color_buffer;
    glGenBuffers(1, &color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_vectors)*3, 
        color_vectors, GL_STATIC_DRAW
    );

    GLuint vposition_buffer;
    glGenBuffers(1, &vposition_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vposition_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bucephalus_vectors)*3,
        bucephalus_vectors, GL_STATIC_DRAW
    );

    GLuint element_buffer;
    glGenBuffers(1, &element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bucephalus_indices),
        bucephalus_indices, GL_STATIC_DRAW
    );
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    /* Shader program initialization logic */
    std::unique_ptr<shaderSrc> vertex_src, fragment_src;
    vertex_src = glhelpers::load_shader_file(
        GL_VERTEX_SHADER, "res/shaders/vert.vert"
    );
    fragment_src = glhelpers::load_shader_file(
        GL_FRAGMENT_SHADER, "res/shaders/frag.frag"
    );
    GLuint vs = glhelpers::compile_shaderSrc(vertex_src.get());
    GLuint fs = glhelpers::compile_shaderSrc(fragment_src.get());

    /* Link the shader program and use it*/
    GLuint shader_prog = glCreateProgram();
    glAttachShader(shader_prog, vs);
    glAttachShader(shader_prog, fs);
    glhelpers::gl_link_program(shader_prog);

    /* Misc. setup for render loop */
    size_t n_elements = sizeof(bucephalus_indices)/sizeof(GLuint);

    int theta = 1;
    int rotational_velocity = 1;
    MovingObject bucephalus(0.0, -1.0, &model[3][0], &model[3][1]);

    glm::mat4 rotation_matrix = (
        glhelpers::rot3d_matrix(theta, 'x') * glhelpers::rot3d_matrix(theta, 'y')
    );
    glUseProgram(shader_prog);
    int model_location = glGetUniformLocation(shader_prog, "model");
    int rot_location = glGetUniformLocation(shader_prog, "rotation");
    int squish_location = glGetUniformLocation(shader_prog, "squish");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(rot_location, 1, GL_FALSE, glm::value_ptr(rotation_matrix));
    glUniformMatrix4fv(squish_location, 1, GL_FALSE, glm::value_ptr(squish));
    glBindBuffer(GL_ARRAY_BUFFER, vposition_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);

    glhelpers::SimpleTimer timer = glhelpers::SimpleTimer();

	const unsigned int FRAME_DELAY_60FPS_CAP = 16500000;

    /* Render loop */
    while (!glfwWindowShouldClose(window)) {
        glhelpers::update_fps_counter(window);
        timer.update();
        
        squish_matrix(squish, model[3][1], 0.15f, -0.6f);
        glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
        rotation_matrix = (
            glhelpers::rot3d_matrix(theta, 'x') * glhelpers::rot3d_matrix(theta, 'y')
        );
        glUniformMatrix4fv(rot_location, 1, GL_FALSE, 
            glm::value_ptr(rotation_matrix)
        );
        glUniformMatrix4fv(squish_location, 1, GL_FALSE, 
            glm::value_ptr(squish)
        );

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        /* Draw objects here */
        glDrawElements(GL_TRIANGLES, n_elements, GL_UNSIGNED_INT, nullptr);

        glfwPollEvents();
        glfwSwapBuffers(window);

        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, 1);
        }

        if (*obj_y > 0.0f || *obj_y < -0.65f) {
            bucephalus.setVelocity(0.0f, -bucephalus.getVelocityY());
        }
        bucephalus.applyVelocity(timer.getElapsedSeconds());
        bucephalus.recordPosition();
        theta = (theta < 360) ? theta + rotational_velocity : 
                                theta + rotational_velocity - 360;

		// cap our program to 60fps with a delay before next frame
		nanoDelay(FRAME_DELAY_60FPS_CAP);
    }

    glfwTerminate();

    return 0;
}
