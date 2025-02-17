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
#include <chrono>
#include <iostream>
#include <fcntl.h>
#include "CgiHandler.hpp"

class ConnectionHandler;
struct clientInfo;

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
		void build500Response(clientInfo *clientPTR);
		void build204Response(clientInfo *clientPTR);
		void buildDirListingResponse(const std::string& pathToDir, clientInfo *ClientPTR);
		void deleteHandler(clientInfo *clientPTR, std::string filePath);
		bool	checkForMultipartFileData(clientInfo *clientPTR);
		void	prepareUploadFile(clientInfo *clientPTR);
		bool 	isValidErrorFile(std::string &errorFileName);

	
    public:
		CgiHandler	*m_cgiHandler = nullptr; // Should this be private and accessed through getter...?
		int openCgiPipes(clientInfo *clientPTR);
        ResponseHandler() = default;
        ResponseHandler(const ResponseHandler& other) = delete;
        const ResponseHandler& operator=(const ResponseHandler& other) = delete;
        ~ResponseHandler() = default;
        void setRequestType(clientInfo *clientPTR);
		void handleRequest(clientInfo *clientPTR);
        void checkFile(clientInfo *clientPTR);
		void setExtension(clientInfo *clientPTR);
		const enum requestTypes& getRequestType() const;
		unsigned int getResponseCode() const;
		bool checkRequestAllowed(clientInfo *clientPTR);
		void buildRedirectResponse(std::string filePath, clientInfo *clientPTR);
		void buildErrorResponse(clientInfo *ClientPTR); // is this ok in public...? used to be in private! - Panu
		void openErrorResponseFile(clientInfo *clientPTR);
		int buildResponse(clientInfo *clientPTR);
		void setResponseCode(unsigned int code); // Added this to public, might be a problem
		int	openResponseFile(clientInfo *clientPTR, std::string filePath);
};

