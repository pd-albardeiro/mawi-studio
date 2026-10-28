cmake_minimum_required(VERSION 3.16)
project(mawi_studio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(GLX REQUIRED glx)
pkg_check_modules(EPOXY REQUIRED epoxy)
pkg_check_modules(GL REQUIRED gl)

add_executable(mawi_studio
    src/main.cpp
    src/scene.cpp
    src/project.cpp
    src/renderer.cpp
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/backends/imgui_impl_opengl2.cpp
)
target_include_directories(mawi_studio PRIVATE include vendor/imgui ${X11_INCLUDE_DIRS} ${GLX_INCLUDE_DIRS} ${EPOXY_INCLUDE_DIRS})
target_link_directories(mawi_studio PRIVATE ${X11_LIBRARY_DIRS} ${GLX_LIBRARY_DIRS} ${EPOXY_LIBRARY_DIRS})
target_link_libraries(mawi_studio PRIVATE ${X11_LIBRARIES} ${GLX_LIBRARIES} ${EPOXY_LIBRARIES} ${GL_LIBRARIES})
target_compile_options(mawi_studio PRIVATE -Wall -Wextra -Wpedantic)
set_source_files_properties(
    vendor/imgui/imgui.cpp vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp vendor/imgui/imgui_widgets.cpp
    vendor/imgui/backends/imgui_impl_opengl2.cpp
    PROPERTIES COMPILE_OPTIONS "-O0;-w"
)

install(TARGETS mawi_studio RUNTIME DESTINATION bin)
