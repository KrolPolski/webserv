/* *************************************************************************** */
/*                                                                             */
/*  $$\      $$\           $$\                                                 */
/*  $$ | $\  $$ |          $$ |                                                */
/*  $$ |$$$\ $$ | $$$$$$\  $$$$$$$\   $$$$$$$\  $$$$$$\   $$$$$$\ $$\    $$\   */
/*  $$ $$ $$\$$ |$$  __$$\ $$  __$$\ $$  _____|$$  __$$\ $$  __$$\\$$\  $$  |  */
/*  $$$$  _$$$$ |$$$$$$$$ |$$ |  $$ |\$$$$$$\  $$$$$$$$ |$$ |  \__|\$$\$$  /   */
/*  $$$  / \$$$ |$$   ____|$$ |  $$ | \____$$\ $$   ____|$$ |       \$$$  /    */
/*  $$  /   \$$ |\$$$$$$$\ $$$$$$$  |$$$$$$$  |\$$$$$$$\ $$ |        \$  /     */
/*  \__/     \__| \_______|\_______/ \_______/  \_______|\__|         \_/      */
/*                                                                             */
/*   By: Panu Kangas, Ryan Boudwin, Patrik Lång                                */
/*                                                                             */
/* *************************************************************************** */

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <map>
#include <filesystem>
#include <chrono>
#include <iostream>
#include <fcntl.h>
#include "Types.hpp"
#include "CgiHandler.hpp"

class ConnectionHandler;
struct clientInfo;

class ResponseHandler
{
    private:
        enum requestTypes	requestType;
		std::string 		requestTypeAsString;
        unsigned int 		responseCode;
		std::string 		contentType;

		static const std::map<std::string, std::string> 		extensionTypes;
		static const std::map<const unsigned int, std::string>	errorCodes;

		void 	setRequestType(enum requestTypes reqType);
		void 	build204Response(clientInfo *clientPTR);
		void 	buildDirListingResponse(const std::string& pathToDir, clientInfo *ClientPTR);
		void 	deleteHandler(clientInfo *clientPTR, std::string filePath);
		bool	checkForMultipartFileData(clientInfo *clientPTR);
		void	prepareUploadFile(clientInfo *clientPTR);
		bool 	isValidErrorFile(std::string &errorFileName);
	
    public:
        ResponseHandler() = default;
        ~ResponseHandler() = default;
        ResponseHandler(const ResponseHandler& other) = delete;
        const ResponseHandler& operator=(const ResponseHandler& other) = delete;

		CgiHandler	*m_cgiHandler = nullptr; // Should this be private and accessed through getter...?

		int 	openCgiPipes(clientInfo *clientPTR);
        void 	setRequestType(clientInfo *clientPTR);
		void 	handleRequest(clientInfo *clientPTR);
        void 	checkFile(clientInfo *clientPTR);
		void 	setExtension(clientInfo *clientPTR);

		const enum requestTypes& getRequestType() const;
		unsigned int getResponseCode() const;

		bool	checkRequestAllowed(clientInfo *clientPTR);
		bool	checkRightsOfDirectory(std::string directoryPath, clientInfo *clientPTR);

		void	buildRedirectResponse301(std::string filePath, clientInfo *clientPTR);
		void	buildRedirectResponse307(std::string filePath, clientInfo *clientPTR);
		void	build500Response(clientInfo *clientPTR);
		void 	build201Response(clientInfo *clientPTR, std::string webPathToFile);
		void	openErrorResponseFile(clientInfo *clientPTR);
		void	buildErrorResponse(clientInfo *ClientPTR); // is this ok in public...? used to be in private! - Panu
		int		buildResponse(clientInfo *clientPTR);
		void	setResponseCode(unsigned int code); // Added this to public, might be a problem
		int		openResponseFile(clientInfo *clientPTR, std::string filePath);
};
