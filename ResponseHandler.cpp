#include "ResponseHandler.hpp"
#include <iostream>

void ResponseHandler::checkRequestType(clientInfo *ClientPTR, std::string requestString)
{
    std::cout << "We managed to get to checkRequestType" << std::endl;
    std::cout << requestString << std::endl;
	std::cout << "We should have printed the http request by now" << std::endl;
	if (!requestString.compare(0, 3, "GET"))
	{
		std::cout << "GET request detected woo" << std::endl;
		requestType = GET;
	}
	else if (!requestString.compare(0, 4, "POST"))
	{
		std::cout << "POST request detected woo" << std::endl;
		requestType = POST;
	}
	else if (!requestString.compare(0, 6, "DELETE"))
	{
		std::cout << "DELETE request detected woo" << std::endl;
		requestType = DELETE;
	}
	else
	{
		std::cout << "INVALID request detected woo" << std::endl;
		requestType = INVALID;
		responseCode = 400;
	}
}

int ResponseHandler::checkFile(std::string filePath)
{
    std::cout << "We should check if the file exists" << std::endl;

	return (0);
}

void ResponseHandler::parseRequest(std::string requestString)
{
	switch (requestType)
	{
		case GET:
		{
			std::istringstream stream(requestString);
			std::vector<std::string> reqVec;
			std::string line;
			std::cout <<"\nBeginning line splits" << std::endl;
			while (std::getline(stream, line, '\n'))
			{
				reqVec.push_back(line);
				std::cout << line << std::endl;
			}
			std::cout << "Line splits done" << std::endl;
			std::cout << "Line one is: " << reqVec.at(0) << std::endl;
			std::istringstream streamL1(reqVec.at(0));
			std::string phrase; 
			std::vector<std::string> lineOne;
			std::cout << "\nBeginning phrase split" << std::endl;
			while (std::getline(streamL1, phrase, ' '))
			{
				lineOne.push_back(phrase);
				std::cout << phrase << std::endl;
			}
			break;
		}	
		default:
			std::cout << "unhandled parseRequest" << std::endl;
	}
}