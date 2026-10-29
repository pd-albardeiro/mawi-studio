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
    src/workspace.cpp
    src/home.cpp
    src/settings.cpp
    src/brand.cpp
    src/mesh_io.cpp
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
target_compile_definitions(mawi_studio PRIVATE MAWI_ICON_FONT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/fa-solid-900.ttf")
set_source_files_properties(
    vendor/imgui/imgui.cpp vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp vendor/imgui/imgui_widgets.cpp
    vendor/imgui/backends/imgui_impl_opengl2.cpp
    PROPERTIES COMPILE_OPTIONS "-O0;-w"
)

install(TARGETS mawi_studio RUNTIME DESTINATION bin)

include(CTest)
if(BUILD_TESTING)
    add_executable(workspace_test tests/workspace_test.cpp src/workspace.cpp src/project.cpp src/scene.cpp)
    target_include_directories(workspace_test PRIVATE include)
    target_compile_options(workspace_test PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME workspace COMMAND workspace_test)
    add_executable(mesh_test tests/mesh_test.cpp src/mesh_io.cpp src/project.cpp src/scene.cpp)
    target_include_directories(mesh_test PRIVATE include)
    target_compile_options(mesh_test PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME mesh COMMAND mesh_test)
endif()
