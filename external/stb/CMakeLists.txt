cmake_minimum_required(VERSION 3.16)
project("stb")

file(GLOB stb_files CONFIGURE_DEPENDS ./src/*.cpp ./include/*.h)

source_group(stb FILES ${stb_files})

add_library(${PROJECT_NAME} STATIC
    ${stb_files}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)