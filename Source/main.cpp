

#include "VulkanCore/VulkanApp.hpp"

//std
#include <iostream>
#include <exception>

int main()
{

	try
	{
		cvr::VulkanApp engine{};
		engine.run(); 
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl; 
		return EXIT_FAILURE; 
	}

	return EXIT_SUCCESS; 
}