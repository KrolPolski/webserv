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

#include <iostream>
#include <map>
#include "Types.hpp"
#include "ResponseHandler.hpp"
#include "ConfigurationHandler.hpp"

struct serverInfo
{
	int			fd;
	ConfigurationHandler *serverConfig;

	serverInfo(int serverFd, ConfigurationHandler *config) : fd(serverFd), serverConfig(config)
	{
	}
};

struct requestParseInfo
{
	bool 		isCgi = false;
	CgiTypes	cgiType = NONE;

	std::map<std::string, std::string>	headerMap; // All headers as key value pairs
	std::string		startLine; // Whole starting line
	std::string		rawContent; // The body of request, if there is any

	std::string	method; // GET POST etc
	std::string filePath; // Relative requested path
	std::string	extension; // has the first . included (for example '.php')
	std::string queryString; // Everything from the URI after '?' character
};

struct clientInfo
{
	serverInfo			*relatedServer = nullptr;
	requestParseInfo	parsedRequest;

	ResponseHandler		*respHandler = nullptr;

	clientStatus		status = RECIEVE_REQUEST;
	clientRequestType	reqType = UNDEFINED;

	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> curTime;

	unsigned int	reqBodyLen = 0;
	long int		reqBodyDataRead = 0;
	long int		bytesToWriteInCgi = -1;
	long int		bytesReceivedFromCgi = 0;
	bool 			bodyOK = false;
	bool			chunkedOK = false;
	bool			reqLenSet = false;

	int		clientFd;
	int		errorFileFd = -1;
	int		responseFileFd = -1;
	int		pipeToCgi[2] = {-1, -1};
	int		pipeFromCgi[2] = {-1, -1};
	int		bytesSent = 0;

	int		clientTimeOutLimit = 10; // Needs to be bigger in Valgrind tests

	bool	stateFlags[9] = {};

	std::string	clientIP;
	std::string	requestString;
	std::string responseBody;
	std::string responseString;

	std::string multipartBoundaryStr;
	std::string	uploadFileName;
	std::string uploadWebPath;
	size_t		multipartFileDataStartIdx;
	int			uploadFileFd = -1;

	clientInfo(int clientFd, serverInfo *defaultServer) : clientFd(clientFd)
	{
		startTime = std::chrono::high_resolution_clock::now();
		respHandler = new ResponseHandler;
		relatedServer = defaultServer;
	}
};
