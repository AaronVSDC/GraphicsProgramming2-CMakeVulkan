//Visual leak detector
#ifdef _DEBUG
#include <vld.h>
#endif



//std
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "Application.h"

int main() 
{

	cve::Application app; 

	try 
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE; 
	}

	return EXIT_SUCCESS; 
}