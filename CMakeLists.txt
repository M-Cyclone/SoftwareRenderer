cmake_minimum_required(VERSION 3.16)

project("software_renderer")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)


find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${EXTERNAL_DIR}/glfw)

add_subdirectory(${EXTERNAL_DIR}/glad)

add_subdirectory(${EXTERNAL_DIR}/stb)

set(TBB_PATH ${EXTERNAL_DIR}/tbb)
set(TBB_DIR "${TBB_PATH}/lib/cmake/tbb")
find_package(TBB REQUIRED)


set(CORE_DIR ${ROOT_DIR}/core)
set(UTILS_DIR ${ROOT_DIR}/utils)
set(RASTERIZER_DIR ${ROOT_DIR}/rasterizer)
set(GEOMETRY_DIR ${ROOT_DIR}/geometry)

file(GLOB core_files CONFIGURE_DEPENDS ${CORE_DIR}/*.h ${CORE_DIR}/*.cpp)
file(GLOB utils_files CONFIGURE_DEPENDS ${UTILS_DIR}/*.hpp ${UTILS_DIR}/*.h ${UTILS_DIR}/*.cpp)
file(GLOB rasterizer_files CONFIGURE_DEPENDS ${RASTERIZER_DIR}/*.hpp ${RASTERIZER_DIR}/*.h ${RASTERIZER_DIR}/*.cpp)
file(GLOB geometry_files CONFIGURE_DEPENDS ${GEOMETRY_DIR}/*.hpp ${GEOMETRY_DIR}/*.h ${GEOMETRY_DIR}/*.cpp)

source_group(Core FILES ${core_files})
source_group(Utils FILES ${utils_files})
source_group(Rasterizer FILES ${rasterizer_files})
source_group(Geometry FILES ${geometry_files})

add_executable(${PROJECT_NAME}
    ${core_files}
    ${utils_files}
    ${rasterizer_files}
    ${geometry_files}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC ${ROOT_DIR}
    PRIVATE ${EXTERNAL_DIR}/glm
)
target_link_libraries(${PROJECT_NAME}
    glfw
    glad
    stb
    TBB::tbb
)

# set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying tbb dll to build folder."
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${TBB_PATH}/redist/intel64/vc14
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)