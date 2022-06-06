cmake_minimum_required(VERSION 3.4)

project(xcb_opengl_simple_program LANGUAGES C)

add_executable(main "main.c" "opengl3.c")
target_include_directories(main PRIVATE "glad/include")
target_sources(main PRIVATE "glad/src/gl.c" "glad/src/glx.c")
target_link_libraries(main "xcb" "X11" "X11-xcb" "xcb-sync" "dl")