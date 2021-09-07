SET(SOURCE_FILES image_cube.cpp)

ADD_EXECUTABLE(entrypoint ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(entrypoint glfw)
TARGET_LINK_LIBRARIES(entrypoint glm)
TARGET_LINK_LIBRARIES(entrypoint OpenGL::GL)
TARGET_LINK_LIBRARIES(entrypoint gl3w)
TARGET_LINK_LIBRARIES(entrypoint gldebug)
TARGET_LINK_LIBRARIES(entrypoint glhelpers)
