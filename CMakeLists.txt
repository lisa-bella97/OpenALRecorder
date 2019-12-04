cmake_minimum_required(VERSION 3.14)
project(OpenALRecorder)

set(CMAKE_CXX_STANDARD 17)

find_path(OPENAL_INCLUDE_DIR al.h
        HINTS
        ENV OPENALDIR
        PATH_SUFFIXES include/AL include/OpenAL include AL OpenAL
        PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /opt
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\1.00.0000;InstallDir]
        )

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_OpenAL_ARCH_DIR libs/Win64)
else()
    set(_OpenAL_ARCH_DIR libs/Win32)
endif()

find_library(OPENAL_LIBRARY
        NAMES OpenAL al openal OpenAL32
        HINTS
        ENV OPENALDIR
        PATH_SUFFIXES libx32 lib64 lib libs64 libs ${_OpenAL_ARCH_DIR}
        PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /opt
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\1.00.0000;InstallDir]
        )

unset(_OpenAL_ARCH_DIR)
mark_as_advanced(OPENAL_LIBRARY OPENAL_INCLUDE_DIR)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_executable(openALRecorder ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/OpenALRecorder.cpp)
target_link_libraries(openALRecorder openal)
