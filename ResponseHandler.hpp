#pragma once
#include <string>
#include "Types.hpp"

class ResponseHandler
{
    private:
        unsigned int requestType;
        unsigned int responseCode;
    public:
        ResponseHandler() = default;
        ResponseHandler(const ResponseHandler& other) = delete;
        const ResponseHandler& operator=(const ResponseHandler& other) = delete;
        ~ResponseHandler() = default;
        int checkRequestType(clientInfo * ClientPTR, std::string requestString);
        int checkFile(std::string filePath);
};