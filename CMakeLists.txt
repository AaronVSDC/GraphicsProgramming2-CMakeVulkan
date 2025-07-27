cmake_minimum_required (VERSION 3.27)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


project(VulkanApp_GP2_AaronVanSichemDeCombe VERSION 0.0.1) 


#--------------------------------------------------------------------------------------
#Use FetchContent to download from github directly and also configure library
#--------------------------------------------------------------------------------------
include(FetchContent)

#GLFW library
FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git 
	GIT_TAG 3.4 
)
FetchContent_MakeAvailable(glfw)

#GLM library
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1 
)
FetchContent_MakeAvailable(glm) 

#STB library (image loader)
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)
# Populate stb repo
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

#Find the vulkan package
find_package(Vulkan REQUIRED)

# Define the executable target
add_executable(VulkanApp_GP2_AaronVanSichemDeCombe 
    "Source/VulkanCore/VulkanApp.cpp"
    "Source/main.cpp"
    "Source/VulkanCore/InitVulkan.cpp"
 "Source/Window/Window.cpp" "Source/VulkanCore/Instance.cpp")
#----------------------------
#LINK WITH LIBRARYS
#----------------------------

#link with Vulkan library
target_link_libraries(VulkanApp_GP2_AaronVanSichemDeCombe ${Vulkan_LIBRARIES} )
target_include_directories(VulkanApp_GP2_AaronVanSichemDeCombe PRIVATE ${Vulkan_INCLUDE_DIRS})

#Link with GLFW 
target_link_libraries(VulkanApp_GP2_AaronVanSichemDeCombe glfw)

#Link with GLM
target_link_libraries(VulkanApp_GP2_AaronVanSichemDeCombe glm)

#Link with STB 
target_link_libraries(VulkanApp_GP2_AaronVanSichemDeCombe stb)

#Link with assimp
target_link_libraries(VulkanApp_GP2_AaronVanSichemDeCombe assimp)

#----------------------------
#Texture/OBJ configuration
#----------------------------
# Copy the entire Textures and Models folder from the source tree into the build tree
file(COPY
     "${CMAKE_SOURCE_DIR}/Textures"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
)

file(COPY
     "${CMAKE_SOURCE_DIR}/Models"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}" 
)

#----------------------------
#Shader configuration
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
