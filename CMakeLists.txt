cmake_minimum_required(VERSION 3.28)
project(3DRenderer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
    set(CMAKE_CXX_FLAGS_DEBUG "/Od")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    # additional warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        # Оставил -msse2 и -O2 и еще два флага, которые не дложны предплагать поддержку чего-то выше sse2.
        # set(CMAKE_CXX_FLAGS_RELEASE "-O2 -march=native -mtune=native -msse2")
        # Для демонстрации буду использовать полный вариант, конечно.
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -fomit-frame-pointer -fno-semantic-interposition -ftree-vectorize -flto -unroll-loops -mavx2 -mbmi -mbmi2 -mpopcnt -mlzcnt -mfma -mabm")
    else()
        # Не уверен, что остальные флаги не gcc-specific.
        set(CMAKE_CXX_FLAGS_RELEASE "-O2 -msse2")
    endif()
endif()
message(CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE})


include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

SET(SOURCES
    src/main.cpp
    src/model/camera.cpp
    src/model/color.cpp
    src/model/rasterizer.cpp
    src/model/object.cpp
    src/model/renderer.cpp
    src/model/world.cpp
    src/model/frame.cpp
    src/model/zbuffer.cpp
    src/model/spectator.cpp
    src/application.cpp
    src/utility/except.cpp
    src/utility/cmd_parser.cpp
    src/model/utility/object_util.cpp
    src/model/utility/simple_obj_parse.cpp)

add_executable(renderer ${SOURCES})
target_link_libraries(renderer PRIVATE sfml-graphics)

if(WIN32)
    add_custom_command(
        TARGET renderer
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:main>
        VERBATIM)
endif()
