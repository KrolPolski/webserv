#include "ResponseHandler.hpp"
#include <iostream>

int ResponseHandler::checkRequestType(clientInfo *ClientPTR, std::string requestString)
{
    std::cout << "We managed to get to checkRequestType" << std::endl;
    exit(0);
}

int ResponseHandler::checkFile(std::string filePath)
{
    std::cout << "We should check if the file exists" << std::endl;
}