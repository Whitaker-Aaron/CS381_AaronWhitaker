cmake_minimum_required(VERSION 3.18)
project(as4 LANGUAGES CXX)
add_subdirectory(raylib-cpp)
file(GLOB sources "src/*.cpp" "src/*.c")
add_executable(as4 ${sources})
target_link_libraries(as4 PUBLIC raylib raylib_cpp raygui)
set_property(TARGET as4 PROPERTY CXX_STANDARD 20)

configure_file(textures/skybox.png textures/skybox.png COPYONLY)
configure_file(textures/deepbluespace.png textures/deepbluespace.png COPYONLY)

configure_file(textures/grass.jpg textures/grass.jpg COPYONLY)
configure_file(textures/ice.jpg textures/ice.jpg COPYONLY)
configure_file(textures/ice2.jpg textures/ice2.jpg COPYONLY)


