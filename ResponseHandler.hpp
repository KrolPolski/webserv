#pragma once
#include <string>
#include <sstream>
#include <vector>
#include "Types.hpp"

class ResponseHandler
{
    private:
        enum requestTypes requestType;
        unsigned int responseCode;
    public:
        ResponseHandler() = default;
        ResponseHandler(const ResponseHandler& other) = delete;
        const ResponseHandler& operator=(const ResponseHandler& other) = delete;
        ~ResponseHandler() = default;
        void checkRequestType(clientInfo *ClientPTR, std::string requestString);
		void parseRequest(std::string requestString);
        int checkFile(std::string filePath);
};