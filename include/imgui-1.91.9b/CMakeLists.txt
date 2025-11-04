##
## Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB IMGUI_INC "./*.h")
file(GLOB IMGUI_SRC "./*.cpp")

add_library (imgui ${IMGUI_INC} ${IMGUI_SRC} "backends/imgui_impl_opengl3.cpp" "backends/imgui_impl_glfw.cpp"
)
target_link_libraries(imgui PRIVATE glfw)
target_include_directories(imgui PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
$<INSTALL_INTERFACE:include>
)