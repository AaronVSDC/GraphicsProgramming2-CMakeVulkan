

#include "VulkanEngine.hpp"

//std
#include <iostream>
#include <exception>

int main()
{

	try
	{
		cve::VulkanEngine engine{};
		engine.run(); 
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl; 
		return EXIT_FAILURE; 
	}

	return EXIT_SUCCESS; 
}