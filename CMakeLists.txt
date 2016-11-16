cmake_minimum_required(VERSION 3.5.2)
project(ios_minicap)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -g")

set(SOURCE_FILES
        src/minicap.cpp
        src/SimpleServer.cpp src/SimpleServer.hpp
        src/FrameListener.cpp src/FrameListener.hpp
        src/Banner.cpp src/Banner.hpp
        src/JpegEncoder.cpp src/JpegEncoder.hpp
        src/StreamClient.mm src/StreamClient.h
        src/Frame.hpp)

add_executable(ios_minicap ${SOURCE_FILES})

include_directories(
        /usr/local/opt/jpeg-turbo/include
)

target_link_libraries (ios_minicap
        "-framework Foundation"
        "-framework CoreFoundation"
        "-framework CoreMedia"
        "-framework CoreVideo"
        "-framework CoreMediaIO"
        "-framework AVFoundation"
        /usr/local/opt/jpeg-turbo/lib/libturbojpeg.a)
