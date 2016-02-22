project(audiodecoder.stsound)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(platform REQUIRED)

include_directories(${kodiplatform_INCLUDE_DIRS}
                    ${platform_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/lib/stsound/StSoundLibrary)

if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
  add_definitions(-D_LINUX)
endif()

set(STSOUND_SOURCES src/YMCodec.cpp
                    lib/stsound/StSoundLibrary/digidrum.cpp
                    lib/stsound/StSoundLibrary/LZH/LzhLib.cpp
                    lib/stsound/StSoundLibrary/Ym2149Ex.cpp
                    lib/stsound/StSoundLibrary/Ymload.cpp
                    lib/stsound/StSoundLibrary/YmMusic.cpp
                    lib/stsound/StSoundLibrary/YmUserInterface.cpp)

set(DEPLIBS ${platform_LIBRARIES})

build_addon(audiodecoder.stsound STSOUND DEPLIBS)

include(CPack)
