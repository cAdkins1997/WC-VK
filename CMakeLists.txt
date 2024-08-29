cmake_minimum_required(VERSION 3.28)
project(WCVK)

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME} source/main.cpp
        source/vkcommon.h
        source/vkinit.h
        source/context.cpp
        source/context.h
        source/application.cpp
        source/application.h
        source/pipelines/descriptors.cpp
        source/pipelines/descriptors.h
        source/vkinit.cpp
        source/pipelines/graphicspipelines.cpp
        source/pipelines/graphicspipelines.h
        source/pipelines/shaders.cpp
        source/pipelines/shaders.h
        source/device/device.cpp
        source/device/device.h
        source/vkcommon.cpp
        source/device/InstanceHelper.cpp
        source/device/InstanceHelper.h
        source/device/DeviceHelper.cpp
        source/device/DeviceHelper.h
        source/device/resources.h
        source/meshes/mesh.h)

if (WIN32)

find_package(Vulkan REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED HINTS "C:/Users/ulver/.vcpkg-clion/vcpkg/packages/vk-bootstrap_x64-mingw-dynamic")
find_package(fmt CONFIG REQUIRED HINTS "C:/Users/ulver/.vcpkg-clion/vcpkg/packages/fmt_x64-mingw-dynamic")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(include/glfw-3.4)

target_link_libraries(${PROJECT_NAME} glfw
        Vulkan::Vulkan
        vk-bootstrap::vk-bootstrap
        vk-bootstrap::vk-bootstrap-compiler-warnings
        fmt::fmt)

if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else()
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/shaders/*.frag"
        "${PROJECT_SOURCE_DIR}/shaders/*.vert"
        "${PROJECT_SOURCE_DIR}/shaders/*.comp"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    message(STATUS "BUILDING SHADER")
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
    message(STATUS ${GLSL})
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/shaders"
        "${PROJECT_SOURCE_DIR}/shaders"
)

elseif (UNIX AND NOT APPLE)
    find_package(Vulkan REQUIRED)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(include/glfw-3.4)

    add_subdirectory(include/fastgltf-0.8.0)

    find_package(vk-bootstrap CONFIG REQUIRED HINTS "${VCPK_ROOT}/vcpkg/packages/vk-bootstrap_x64-linux")

    target_link_libraries(${PROJECT_NAME}
            Vulkan::Vulkan
            glfw
            vk-bootstrap::vk-bootstrap vk-bootstrap::vk-bootstrap-compiler-warnings
            fastgltf::fastgltf
    )

    set(GLSL_VALIDATOR "/${VULKAN_SDK}bin/glslangValidator")

    file(GLOB_RECURSE GLSL_SOURCE_FILES
            "${PROJECT_SOURCE_DIR}/shaders/*.frag"
            "${PROJECT_SOURCE_DIR}/shaders/*.vert"
            "${PROJECT_SOURCE_DIR}/shaders/*.comp"
    )

    foreach(GLSL ${GLSL_SOURCE_FILES})
        message(STATUS "BUILDING SHADER")
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
        message(STATUS ${GLSL})
        add_custom_command(
                OUTPUT ${SPIRV}
                COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
                COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
                DEPENDS ${GLSL})
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach(GLSL)

    add_custom_target(
            Shaders
            DEPENDS ${SPIRV_BINARY_FILES}
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/shaders/"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/shaders"
            "${PROJECT_SOURCE_DIR}/shaders"
    )

endif ()