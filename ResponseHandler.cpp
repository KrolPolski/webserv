#include "ResponseHandler.hpp"
#include <iostream>

enum requestTypes ResponseHandler::checkRequestType(clientInfo *ClientPTR, std::string requestString)
{
    std::cout << "We managed to get to checkRequestType" << std::endl;
    std::cout << requestString << std::endl;
	std::cout << "We should have printed the http request by now" << std::endl;
	if (!requestString.compare(0, 3, "GET"))
	{
		std::cout << "GET request detected woo" << std::endl;
		return (GET);
	}
	else if (!requestString.compare(0, 4, "POST"))
	{
		std::cout << "POST request detected woo" << std::endl;
		return (POST);
	}
	else if (!requestString.compare(0, 6, "DELETE"))
	{
		std::cout << "DELETE request detected woo" << std::endl;
		return (DELETE);
	}
	else
	{
		std::cout << "INVALID request detected woo" << std::endl;
		return (INVALID);}
}

int ResponseHandler::checkFile(std::string filePath)
{
    std::cout << "We should check if the file exists" << std::endl;
	return (0);
}