find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

link_directories(${CMAKE_SOURCE_DIR}/main/libs/)

include_directories(./includes/)
include_directories(./glad/include/)

include_directories(../../../sdk/emsdk/upstream/emscripten/system/include/)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    option(BUILD_HTML "Building html..." ON)
    set(EMSCRIPTEN TRUE)
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/camera.cpp
    src/ray.cpp
    src/gl_shader_program.cpp
    src/gl_surface.cpp
    src/window.cpp
    glad/src/glad.c
    src/image.cpp
    src/timer.cpp
    src/mesh.cpp
    src/software_renderer.cpp
    src/transform.cpp

    includes/camera.h
    includes/glfw3.h
    includes/glfw3native.h
    includes/gl_shader_program.h
    includes/gl_surface.h
    includes/stb_image.h
    includes/transform.h
    includes/window.h
    glad/include/glad/glad.h
    glad/include/KHR/khrplatform.h
    includes/stb_image_write.h
    includes/image.h
    includes/timer.h
    includes/mesh.h  
    includes/shader_program.h
    includes/simple_shader.h
    includes/debug_light_shader.h
    includes/blinn_phong_shader.h
    includes/shadowmap_shader.h
    includes/software_renderer.h
)

set(LIBS glfw3 ersatz)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})

if (EMSCRIPTEN)   
    add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/src/resources ${CMAKE_CURRENT_BINARY_DIR}/resources)
else()
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/src/resources ${CMAKE_CURRENT_BINARY_DIR}/Release/resources)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/src/resources ${CMAKE_CURRENT_BINARY_DIR}/Debug/resources)
endif()

if (EMSCRIPTEN)   
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -s FULL_ES3=1 -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s ASYNCIFY_IMPORTS=[open_tab] --preload-file resources")
    if (BUILD_HTML)
        set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    endif() 
elseif (MSVC)
    set(CMAKE_CXX_FLAGS "/W4 ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "/wd4996 ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "-Wno-deprecated ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
endif()