cmake_minimum_required(VERSION 3.29)

project(AaronVanSichemDeCombe_GP2_VulkanApp VERSION 0.0.1)

#Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

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

#Find the vulkan package
find_package(Vulkan REQUIRED)

#Add subdirectories
add_subdirectory(PhysicsSandbox)