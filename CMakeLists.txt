cmake_minimum_required(VERSION 3.27)

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


FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.2
)
FetchContent_MakeAvailable(assimp) 
#create interface target
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

find_package(Vulkan REQUIRED)

# Source files
set(SOURCE_FILES
  source/main.cpp
  "Source/App/Core/Application.cpp"
  "Source/App/Window/Window.cpp"
  "Source/Vulkan/Pipeline/Pipeline.cpp"
  "Source/Vulkan/Device/Device.cpp"
  "Source/Vulkan/Swapchain/SwapChain.cpp"
  "Source/App/ModelLoading/Model.cpp"
  "Source/App/Core/GameObject.h"
  "Source/App/Renderer/Renderer.cpp"
  "Source/App/Renderer/SimpleRenderSystem.cpp"
  "Source/App/Camera/Camera.cpp"
  "Source/App/UserInput/UserInput.cpp"
  "Source/App/Utils/Utils.h"
  "Source/Vulkan/Textures/Texture.cpp"
  "Source/App/GBuffer/GBuffer.cpp"
)

# Define executable
add_executable(${TARGET_NAME} ${SOURCE_FILES} "Source/App/GBuffer/GBuffer.cpp")

# Link libraries
target_link_libraries(${TARGET_NAME}
  PRIVATE ${Vulkan_LIBRARIES}
  glfw 
  glm
  assimp
)
#------------------------------------------------------------------------------
# Auto–collect all subdirectories under source/ that contain .h files
#------------------------------------------------------------------------------
file(GLOB_RECURSE HEADER_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h"
)

set(HEADER_DIRS "")
foreach(HDR ${HEADER_FILES})
  get_filename_component(HDR_DIR ${HDR} DIRECTORY)
  list(APPEND HEADER_DIRS ${HDR_DIR})
endforeach()
list(REMOVE_DUPLICATES HEADER_DIRS)

# Tell our target to include _every_ header folder
target_include_directories(${TARGET_NAME}
  PRIVATE
    ${HEADER_DIRS}
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
set(MODELS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
add_custom_command(
  TARGET ${TARGET_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${MODELS_SOURCE_DIR}"
    "$<TARGET_FILE_DIR:${TARGET_NAME}>/Resources"
)

