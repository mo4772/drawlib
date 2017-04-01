project(drawlib)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_FLAGS g++)
set(CMAKE_CXX_FLAGS g++)

# source files
file(GLOB srcfiles "./*.cpp" "./baselib/*.cpp" "./gltool/*.cpp")

# include direcotry
include_directories(. ./baselib/ ./gltool ./soil ./boost /usr/include/GL /usr/local/include/GLFW /usr/local/include/freetype2)

add_definitions(-DGLFW_EXPOSE_NATIVE_X11)


set(CMAKE_CXX_FLAGS "-O2 -g -Wall -Wno-write-strings -rdynamic -fpermissive -lm")
set(CMAKE_C_FLAGS "-O2 -g -Wall -fPIC -W -Wno-write-strings -rdynamic -fpermissive -lm")

set(CMAKE_BUILD_TYPE=Debug)

add_library(drawlib STATIC ${srcfiles})
