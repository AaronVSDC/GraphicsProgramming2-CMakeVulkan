cmake_minimum_required (VERSION 3.27)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


project(VulkanApp_GP2_AaronVanSichemDeCombe VERSION 0.0.1) 


#--------------------------------------------------------------------------------------
#Use fancy FetchContent to download from github directly and also configure library
#--------------------------------------------------------------------------------------
include(FetchContent)

#GLFW library
FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git 
	GIT_TAG 3.4 # Specify the version you need
)
FetchContent_MakeAvailable(glfw)

#GLM library
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1 # Specify the version you want to use
)
FetchContent_MakeAvailable(glm)

#Tiny_Obj_Loader library
#FetchContent_Declare(
#  tinyobjloader
#  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
#  GIT_TAG        v2.0.0rc5  # Specify the version you need
#)
#FetchContent_MakeAvailable(tinyobjloader)

#Find the vulkan package
find_package(Vulkan REQUIRED)

# Define the executable target
add_executable(VulkanApp_GP2_AaronVanSichemDeCombe 
    "Source/VulkanEngine.cpp"
    "Source/main.cpp"
    "Source/InitVulkan.cpp"
)

#----------------------------
#Visual Leak Detector
#----------------------------
#if(MSVC)
#    find_path(VLD_INCLUDE_DIR NAMES vld.h PATHS "C:/Program Files (x86)/Visual Leak Detector/include")
#    find_library(VLD_LIBRARY NAMES vld.lib PATHS "C:/Program Files (x86)/Visual Leak Detector/lib/Win64")
#    include_directories(${VLD_INCLUDE_DIR})
#    target_link_libraries(VulkanApp_GP2_AaronVanSichemDeCombe ${VLD_LIBRARY})
#endif()

#----------------------------
#LINK WITH LIBRARYS
#----------------------------

#link with Vulkan library
target_link_libraries(VulkanApp_GP2_AaronVanSichemDeCombe ${Vulkan_LIBRARIES} )
target_include_directories(VulkanApp_GP2_AaronVanSichemDeCombe PRIVATE ${Vulkan_INCLUDE_DIRS})

#Link with GLFW 
target_link_libraries(VulkanApp_GP2_AaronVanSichemDeCombe glfw)

#Link with Tiny_Object_Loader library
#target_link_libraries(VulkanApp_GP2_AaronVanSichemDeCombe tinyobjloader)


#----------------------------
#Configure shader settings
#----------------------------

#####################################################################################################################################
#Basically all of this stuff is to compile the shader files into .spv binary files so that it can be used in the project (pipeline)
#####################################################################################################################################

# Define directories
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Shaders")

# Get GLSL shader files
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert")

# Find the glslc compiler
find_program(GLSLC_EXECUTABLE NAMES glslc HINTS ${Vulkan_GLSLC_EXECUTABLE})

# Compile GLSL to SPIR-V
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
endforeach(GLSL)

# Create a custom target for shaders
add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

# Add Shader executable dependency
add_dependencies(VulkanApp_GP2_AaronVanSichemDeCombe Shaders)

# Add shader files to your target; they will not be compiled
target_sources(VulkanApp_GP2_AaronVanSichemDeCombe PRIVATE ${GLSL_SOURCE_FILES})

# For each shader file, mark it as HEADER_FILE_ONLY
set_source_files_properties(${GLSL_SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

# Organize shader files in the IDE
source_group("Shaders" FILES ${GLSL_SOURCE_FILES})



if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VulkanApp_GP2_AaronVanSichemDeCombe PROPERTY CXX_STANDARD 20)
endif()
