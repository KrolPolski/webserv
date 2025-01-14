#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <map>
#include "Types.hpp"
#include <filesystem>

class ResponseHandler
{
    private:
        enum requestTypes requestType;
		std::string requestTypeAsString;
        unsigned int responseCode;
		std::string contentType;
		static const std::map<std::string, std::string> extensionTypes;
		static const std::map<const unsigned int, std::string> errorCodes;
		void setRequestType(enum requestTypes reqType);
		void setResponseCode(unsigned int code);
		void buildErrorResponse(clientInfo *ClientPTR);
		void build500Response(clientInfo *clientPTR);
		void buildDirListingResponse(const std::string& pathToDir, clientInfo *ClientPTR);
	
    public:
        ResponseHandler() = default;
        ResponseHandler(const ResponseHandler& other) = delete;
        const ResponseHandler& operator=(const ResponseHandler& other) = delete;
        ~ResponseHandler() = default;
        void checkRequestType(clientInfo *ClientPTR, std::string requestString);
		void parseRequest(clientInfo *ClientPTR, std::string requestString);
		void ServeErrorPages(clientInfo *ClientPTR, std::string requestString);
        int checkFile(clientInfo *ClientPTR, std::string filePath);
		void checkExtension(std::string filePath);
		const enum requestTypes& getRequestType() const;
		unsigned int getResponseCode() const;
		bool checkRequestAllowed(clientInfo *clientPTR, std::string filePath);
		void buildRedirectResponse(std::string filePath, clientInfo *clientPTR);
};

