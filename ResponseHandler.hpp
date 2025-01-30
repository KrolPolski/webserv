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
#include "CgiHandler.hpp"

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
		void buildDirListingResponse(const std::string& pathToDir, clientInfo *ClientPTR);
		bool	checkForMultipartFileData(clientInfo *clientPTR);
		void	prepareUploadFile(clientInfo *clientPTR);
	
    public:
		CgiHandler	*m_cgiHandler = nullptr; // Should this be private and accessed through getter...?
		int openCgiPipes(clientInfo *clientPTR);


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
		void buildErrorResponse(clientInfo *ClientPTR); // is this ok in public...? used to be in private! - Panu
		void openErrorResponseFile(clientInfo *clientPTR);
		int buildResponse(clientInfo *clientPTR);
		
		void setResponseCode(unsigned int code); // Added this to public, might be a problem
		int	openResponseFile(clientInfo *clientPTR, std::string filePath);





};

