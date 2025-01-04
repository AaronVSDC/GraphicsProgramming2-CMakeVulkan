//Visual leak detector
#ifdef _DEBUG
#include <vld.h>
#endif

#include "VulkanEngine.hpp"

//std
#include <iostream>
#include <exception>

int main()
{

	cve::VulkanEngine engine{}; 
	try
	{
		engine.run(); 
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl; 
		return EXIT_FAILURE; 
	}

	return EXIT_SUCCESS; 
}