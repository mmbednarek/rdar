cmake_minimum_required(VERSION 3.16)
project(rdar)

set(CMAKE_CXX_STANDARD 17)
# SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
# SET(BUILD_SHARED_LIBS OFF)
# SET(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_EXE_LINKER_FLAGS  "-static-libgcc -static-libstdc++ -Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -Wl,--enable-auto-import")

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 7.1.3
)
FetchContent_MakeAvailable(fmt)

add_subdirectory(./src/libww)

add_executable(rdar src/main.cpp src/archive.h src/reader.cpp src/reader.h src/util.h src/util.cpp src/archive.cpp src/file_sink.cpp src/file_sink.h)
target_link_libraries(rdar LINK_PUBLIC fmt libww)