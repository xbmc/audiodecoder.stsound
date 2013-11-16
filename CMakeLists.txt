project(audiodecoder.ym)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(xbmc REQUIRED)

include_directories(${XBMC_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/lib/stsound/StSoundLibrary)

add_definitions(-D_LINUX)

set(YM_SOURCES src/YMCodec.cpp
               lib/stsound/StSoundLibrary/Depacker.cpp
               lib/stsound/StSoundLibrary/digidrum.cpp
               lib/stsound/StSoundLibrary/LZH/lzhxlib.c
               lib/stsound/StSoundLibrary/Ym2149Ex.cpp
               lib/stsound/StSoundLibrary/Ymload.cpp
               lib/stsound/StSoundLibrary/YmMusic.cpp
               lib/stsound/StSoundLibrary/YmUserInterface.cpp)

build_addon(audiodecoder.ym YM DEPLIBS)
