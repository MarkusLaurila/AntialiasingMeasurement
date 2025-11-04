cmake_minimum_required(VERSION 3.10)
project(antialiasing)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLFW_BUILD_SHARED_LIBS OFF CACHE BOOL "GLFW static" FORCE)
set(GLEW_BUILD_SHARED OFF CACHE BOOL "GLEW shared OFF" FORCE)
set(GLEW_BUILD_STATIC ON CACHE BOOL "GLEW static ON" FORCE)
set(FFTW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/fftw")
set(FFTW_LIB_DOUBLE "${CMAKE_CURRENT_SOURCE_DIR}/include/fftw/libfftw3-3.a")
set(FFTW_LIB_FLOAT "${CMAKE_CURRENT_SOURCE_DIR}/include/fftw/libfftw3f-3.a")
find_package(OpenGL REQUIRED)
set(ASSIMP_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_BLEND_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER OFF CACHE BOOL "" FORCE)
add_subdirectory(include/assimp EXCLUDE_FROM_ALL)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)
add_subdirectory(include/glfw-3.4)
add_subdirectory(include/glew-2.1.0/build/cmake)
add_subdirectory(include/glm)
add_subdirectory(include/soil-master)
add_subdirectory(include/imgui-1.91.9b)
add_executable(antialiasing
        source/main.cpp
        source/Shader.cpp
        source/Display.cpp
        source/ImageCapture.cpp
        source/OverLay.cpp
        header/Display.h
        header/Shader.h
        header/ImageCapture.h
        header/OverLay.h
        header/SkyBox.h
        source/SkyBox.cpp
        header/CornerDetector.h
        source/CornerDetector.cpp
        source/ModelImporter.cpp
        header/ModelImporter.h
)
target_include_directories(antialiasing PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/header
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glfw-3.4/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glew-2.1.0/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/soil-master/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui-1.91.9b
        ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui-1.91.9b/backends
        ${FFTW_INCLUDE_DIR}



)
target_link_libraries(antialiasing PRIVATE
        glfw
        glew_s
        glm
        soil
        imgui
        OpenGL::GL
        assimp::assimp
        ${FFTW_LIB_DOUBLE}
        ${FFTW_LIB_FLOAT}


)
add_custom_command(TARGET antialiasing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/include/fftw/libfftw3-3.dll"
        $<TARGET_FILE_DIR:antialiasing>)

add_custom_command(TARGET antialiasing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/include/fftw/libfftw3f-3.dll"
        $<TARGET_FILE_DIR:antialiasing>)

add_custom_command(TARGET antialiasing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:assimp>"
        $<TARGET_FILE_DIR:antialiasing>)

