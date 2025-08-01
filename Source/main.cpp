//std
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <filesystem>

#include "Application.h"

int main() 
{


	try 
	{
		cve::Application app;
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE; 
	}

	return EXIT_SUCCESS; 
}
