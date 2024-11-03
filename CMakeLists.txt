cmake_minimum_required(VERSION 3.20)
project(NextJam VERSION 0.1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(RAYLIB_VERSION 5.5)
add_subdirectory("../raylib" "../raylib/cmake")
add_subdirectory("../box2d" "../box2d/cmake")
add_subdirectory("../LDtkLoader" "../LDtkLoader/cmake")


file(GLOB_RECURSE SOURCE_LIST
    "./include/*.h"
    "src/*.c"
    "src/*.cpp"
    "../raygui/src/*h"
)

foreach(_source IN ITEMS ${SOURCE_LIST})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

include_directories(
    "include"
    "src"
    "../raygui/src"
)

#add_compile_definitions(_DEBUG)
add_compile_definitions(RAYGUI_IMPLEMENTATION)
add_compile_definitions(RAYLIB_ASEPRITE_IMPLEMENTATION)
add_executable(NextJam
    "${SOURCE_LIST}"
)
set_target_properties(NextJam PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(NextJam
    raylib
    box2d
    LDtkLoader
)

if (${PLATFORM} STREQUAL "Web")
    set_target_properties(NextJam PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "--preload-file ../../src/resources --shell-file ../../src/minshell.html"
        COMPILE_FLAGS "-Wno-narrowing"
    )
endif()