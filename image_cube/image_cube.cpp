#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>

#include <math.h>
#include <stdio.h>
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
#include "../include/cube.hpp"
#include "../include/stb_image.hpp"

// structs defined in glhelpers.hpp for convient grouping of things
using glhelpers::displayObjects;
using glhelpers::shaderSrc;

int main() {
    if (!glfwInit()) {
        GL_LOG_ERROR() << "ERROR: could not start GLFW3";
        return 1;
    }

    /* Window hints */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    /* Video mode, window, display */
    std::unique_ptr<displayObjects> display_objects = glhelpers::getDisplayObjects();
    GLFWwindow* window = glhelpers::glfwCreatePrimaryWindow(
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
    float wcorr = glhelpers::WIDESCREEN_SCALING_DIVISOR; // correction factor for widescreen
    glm::mat4 model{
         scale,         0.0f,   0.0f, 0.0f,
          0.0f,  scale/wcorr,   0.0f, 0.0f,
          0.0f,         0.0f,  scale, 0.0f,
          0.0f,         0.0f,   0.0f, 1.0f
    };
    float* obj_x = &model[3][0];
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
    };

    // Load container image into a texture
    int width, height, nrChannels;
    unsigned char *container_img_data = stbi_load("res/img/container.jpg", &width, &height, 
            &nrChannels, 0);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    if (container_img_data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                GL_UNSIGNED_BYTE, container_img_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        printf("Failed to load image\n");
    }
    stbi_image_free(container_img_data);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Binding for our 3d objects */
    unsigned int vertex_arr;
    unsigned int vertex_buffer, color_buffer, element_buffer;

    glGenVertexArrays(1, &vertex_arr);
    glBindVertexArray(vertex_arr);

    glGenBuffers(1, &color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_vectors)*3, 
        color_vectors, GL_STATIC_DRAW
    );

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, glshapes::SIZE_IMAGE_CUBE_VERTICES*5,
        glshapes::IMAGE_CUBE_VERTICES, GL_STATIC_DRAW
    );

    glGenBuffers(1, &element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, glshapes::SIZE_IMAGE_CUBE_INDICES,
        glshapes::IMAGE_CUBE_INDICES, GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, nullptr);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, 
            (void*)(3*sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    /* Shader program initialization logic */
    std::unique_ptr<shaderSrc> vertex_src, fragment_src;
    vertex_src = glhelpers::load_shader_file(
        GL_VERTEX_SHADER, "res/shaders/fifth.vert"
    );
    fragment_src = glhelpers::load_shader_file(
        GL_FRAGMENT_SHADER, "res/shaders/fifth.frag"
    );
    GLuint vs = glhelpers::compile_shaderSrc(vertex_src.get());
    GLuint fs = glhelpers::compile_shaderSrc(fragment_src.get());

    /* Link the shader program and use it*/
    GLuint shader_prog = glCreateProgram();
    glAttachShader(shader_prog, vs);
    glAttachShader(shader_prog, fs);
    glhelpers::gl_link_program(shader_prog);

    /* Misc. setup for render loop */
    size_t n_elements = glshapes::SIZE_IMAGE_CUBE_INDICES/sizeof(unsigned);

    int theta = 1;
    int rotational_velocity = 1;

    glm::mat4 rotation_matrix = (
        glhelpers::rot3d_matrix(theta, 'x') * glhelpers::rot3d_matrix(theta, 'y')
    );
    glUseProgram(shader_prog);
    int model_location = glGetUniformLocation(shader_prog, "model");
    int rot_location = glGetUniformLocation(shader_prog, "rotation");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(rot_location, 1, GL_FALSE, glm::value_ptr(rotation_matrix));
    // glBindBuffer(GL_ARRAY_BUFFER, vposition_buffer);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    // glBindBuffer(GL_ARRAY_BUFFER, cube_data_buffer);
    // glBindVertexArray(vertex_arr);

    glhelpers::SimpleTimer timer = glhelpers::SimpleTimer();

    /* Render loop */
    while (!glfwWindowShouldClose(window)) {
        glhelpers::update_fps_counter(window);
        timer.update();
        
        glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
        glm::mat4 rotation_matrix = (
            glhelpers::rot3d_matrix(theta, 'x') * glhelpers::rot3d_matrix(theta, 'y')
        );
        glUniformMatrix4fv(rot_location, 1, GL_FALSE, 
            glm::value_ptr(rotation_matrix)
        );
        glBindTexture(GL_TEXTURE_2D, texture);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 
                glhelpers::get_glfw_primary_window_width(), 
                glhelpers::get_glfw_primary_window_height());

        /* Draw objects here */
        glDrawElements(GL_TRIANGLES, n_elements, GL_UNSIGNED_INT, nullptr);

        glfwPollEvents();
        glfwSwapBuffers(window);

        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, 1);
        }

        theta = (theta < 360) ? theta + rotational_velocity : 
                                theta + rotational_velocity - 360;
    }

    glfwTerminate();

    return 0;
}
