cmake_minimum_required(VERSION 3.18)
project(as2 LANGUAGES CXX)
add_subdirectory(raylib-cpp)
file(GLOB sources "src/*.cpp" "src/*.c")
add_executable(as2 ${sources})
target_link_libraries(as2 PUBLIC raylib raylib_cpp)
set_property(TARGET as2 PROPERTY CXX_STANDARD 20)

configure_file(textures/skybox.png textures/skybox.png COPYONLY)
configure_file(textures/grass.jpg textures/grass.jpg COPYONLY)
configure_file(textures/ice.jpg textures/ice.jpg COPYONLY)
configure_file(textures/ice2.jpg textures/ice2.jpg COPYONLY)


