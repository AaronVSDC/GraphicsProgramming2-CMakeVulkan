cmake_minimum_required(VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT 
      "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(VulkanApp_GP2_AaronVanSichemDeCombe VERSION 0.0.1)

# Use project name variable for target
set(TARGET_NAME ${PROJECT_NAME})

#--------------------------------------------------------------------------------------
# FetchContent for external libraries
#--------------------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(tinyobjloader
  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
  GIT_TAG v2.0.0rc5
)
FetchContent_MakeAvailable(tinyobjloader)

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

find_package(Vulkan REQUIRED)

# Source files
set(SOURCE_FILES
  source/main.cpp
  source/Application.cpp
  source/Application.h
  source/Window.cpp
  source/Window.h
  source/Pipeline.cpp
  source/Pipeline.h
  source/Device.cpp
  source/Device.h
  source/SwapChain.cpp
  source/SwapChain.h
  source/Model.cpp
  source/Model.h
  source/GameObject.cpp
  source/GameObject.h
  source/Renderer.cpp
  source/Renderer.h
  source/SimpleRenderSystem.cpp
  source/SimpleRenderSystem.h
  source/Camera.cpp
  source/Camera.h
  source/UserInput.cpp
  source/UserInput.h
  source/Utils.cpp
  source/Utils.h
  source/Texture.h
  source/Texture.cpp)

# Define executable
add_executable(${TARGET_NAME} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${TARGET_NAME}
  PRIVATE ${Vulkan_LIBRARIES} glfw tinyobjloader
)
target_include_directories(${TARGET_NAME}
  PRIVATE 
    ${Vulkan_INCLUDE_DIRS}
    ${stb_SOURCE_DIR}
)

# C++20 standard
if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
endif()

#--------------------------------------------------------------------------------------
# Shaders: compile GLSL -> SPIR-V
#--------------------------------------------------------------------------------------
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Shaders")

file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${SHADER_SOURCE_DIR}/*.vert"
  "${SHADER_SOURCE_DIR}/*.frag"
)

find_program(GLSLC_EXECUTABLE NAMES glslc HINTS ${Vulkan_GLSLC_EXECUTABLE})

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}"
    COMMAND ${GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL}
  )
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(${TARGET_NAME} Shaders)
target_sources(${TARGET_NAME} PRIVATE ${GLSL_SOURCE_FILES})
set_source_files_properties(${GLSL_SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("Shaders" FILES ${GLSL_SOURCE_FILES})

#--------------------------------------------------------------------------------------
# Copy Models folder next to the executable (portable, no folder-name assumptions)
#--------------------------------------------------------------------------------------
set(MODELS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Models")
add_custom_command(
  TARGET ${TARGET_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${MODELS_SOURCE_DIR}"
    "$<TARGET_FILE_DIR:${TARGET_NAME}>/Models"
)

#--------------------------------------------------------------------------------------
# Copy Textures folder next to the executable
#--------------------------------------------------------------------------------------
set(TEXTURES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Textures")
add_custom_command(
  TARGET ${TARGET_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${TEXTURES_SOURCE_DIR}"
    "$<TARGET_FILE_DIR:${TARGET_NAME}>/Textures"
)
