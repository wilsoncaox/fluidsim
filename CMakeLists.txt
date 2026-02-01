cmake_minimum_required(VERSION 3.22)

project(app)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
set(GLFW_LIB glfw)

set(TINYOBJ_PATH external/tinyobjloader)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)


add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${TINYOBJ_PATH}
)
target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})

find_program(GLSLC_EXECUTABLE
    NAMES glslc
    HINTS
        /usr/bin
        /usr/local/bin
)

file(GLOB_RECURSE GLSL_SOURCE_FILES CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/shaders/*.frag"
  "${PROJECT_SOURCE_DIR}/shaders/*.vert"
  "${PROJECT_SOURCE_DIR}/shaders/*.comp"
)

set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

set(SPIRV_BINARY_FILES)

foreach(GLSL ${GLSL_SOURCE_FILES})
    file(RELATIVE_PATH REL_PATH ${PROJECT_SOURCE_DIR}/shaders ${GLSL})
    set(SPIRV "${CMAKE_BINARY_DIR}/shaders/${REL_PATH}.spv")
    get_filename_component(SPIRV_DIR ${SPIRV} DIRECTORY)
    file(MAKE_DIRECTORY ${SPIRV_DIR})

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
        COMMENT "Compiling shader ${REL_PATH}"
    )

    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

add_dependencies(${PROJECT_NAME} Shaders)
