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

int ResponseHandler::checkFile(clientInfo *clientPTR, std::string filePath)
{
    std::cout << "We should now check if the file exists" << std::endl;
	// this has to be fixed once we have a configuration parsing appropriately
	if (filePath == "/")
		filePath = "home/index.html";
	else
		filePath = "home" + filePath;
	std::string content;
	std::ifstream ourFile(filePath);
	/*We need to detect which type of error we got with the file, so we 
	can send the appropriate response code*/
	if (!ourFile)
	{
		std::cerr << "Error: " << strerror(errno) << errno << std::endl;
		if (errno == 2) //file missing
			responseCode = 404;
		else if (errno == 13) //bad permissions
			responseCode = 403;
		else
			responseCode = 500;
		std::cerr << "Response Code: " << responseCode << std::endl;
		return -1;
	}
	std::string line;
	while (std::getline(ourFile, line))
		content += line;
	std::string	headers;

	//this assumes all files are html, this is a mistake.
	headers = "HTTP/1.1 200 OK\r\n";
	headers += "Content-Type: text/html\r\n";
	headers += "Content-Length: ";
	headers += std::to_string(content.length());
	headers += "\r\n\r\n";
	clientPTR->responseString = headers + content;
	ourFile.close();
	responseCode = 200;
	std::cout << "Must have opened the file with no errors" << std::endl;
	return (0);
}

void ResponseHandler::parseRequest(clientInfo *clientPTR, std::string requestString)
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
			checkFile(clientPTR, lineOne.at(1));
			break;
		}	
		default:
			std::cout << "unhandled parseRequest" << std::endl;
	}
}