cmake_minimum_required(VERSION 3.29)

project(PhysicsSandbox)

#--------------------------
#Add source files
#--------------------------
set(SOURCE_FILES 
	source/main.cpp
	source/Application.cpp
	source/Application.h
	source/Window.cpp 
	source/Window.h
	source/Pipeline.h
	source/Pipeline.cpp
	source/Device.h 
	source/Device.cpp
	source/SwapChain.h
	source/SwapChain.cpp
	source/Model.h
	source/Model.cpp
	source/GameObject.h
	source/GameObject.cpp
	source/Renderer.h
	source/Renderer.cpp
	source/SimpleRenderSystem.h
	source/SimpleRenderSystem.cpp
	source/Camera.h
	source/Camera.cpp
	source/UserInput.h
	source/UserInput.cpp
	source/Utils.h
	source/Utils.cpp 

)

#Add executable for project
add_executable(PhysicsSandbox ${SOURCE_FILES})

#link with Vulkan library
target_link_libraries(PhysicsSandbox ${Vulkan_LIBRARIES})
target_include_directories(PhysicsSandbox PRIVATE ${Vulkan_INCLUDE_DIRS})

#Link with GLFW
target_link_libraries(PhysicsSandbox glfw)

#Link with Tiny_Object_Loader library
target_link_libraries(PhysicsSandbox tinyobjloader)

# Models directory handling for IDE visibility
file(GLOB_RECURSE MODEL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Models/*.*")
source_group("Models" FILES ${MODEL_FILES})

# Ensure these files are not compiled
set_source_files_properties(${MODEL_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
#----------------------------
#Configure shader settings
#----------------------------
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Shaders")
file(GLOB_RECURSE GLSL_SOURCE_FILES
	"${SHADER_SOURCE_DIR}/*.frag"
	"${SHADER_SOURCE_DIR}/*.vert")

foreach(GLSL ${GLSL_SOURCE_FILES})
	get_filename_component(FILE_NAME ${GLSL} NAME)
	set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}" #this makes sure there is a directory to store in before compiling
		COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
		DEPENDS ${GLSL}
	)
	list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
	Shaders
	DEPENDS ${SPIRV_BINARY_FILES}
)
#Add Shader executable
add_dependencies(${PROJECT_NAME} Shaders)


#Add shader files to your target; they will not be compiled
target_sources(PhysicsSandbox PRIVATE ${GLSL_SOURCE_FILES})

#For each shader file, mark it as HEADER_FILE_ONLY
set_source_files_properties(${GLSL_SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

#Organize shader files in the IDE
source_group("Shaders" FILES ${GLSL_SOURCE_FILES})
