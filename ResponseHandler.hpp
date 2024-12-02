#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cerrno>
#include <cstring>
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
		void parseRequest(clientInfo *ClientPTR, std::string requestString);
        int checkFile(clientInfo *ClientPTR, std::string filePath);
};