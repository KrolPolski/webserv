#include "Types.hpp"
#include "ResponseHandler.hpp"
#include "ConfigurationHandler.hpp"
#include <iostream>
#include <map>

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

	std::string		startLine; // Whole starting line
	std::map<std::string, std::string>	headerMap; // All headers as key value pairs
	std::string		rawContent; // The body of request, if there is any

	std::string	method; // GET POST etc
	std::string filePath; // Relative requested path
	std::string	extension; // has the first . included (for example '.php')
	std::string queryString; // Everything from the URI after '?' character
};

struct clientInfo
{
	const serverInfo	*relatedServer;
	requestParseInfo	parsedRequest;

	ResponseHandler		*respHandler = nullptr;

	clientStatus		status = RECIEVE_REQUEST;
	clientRequestType	reqType = UNDEFINED;

	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> curTime;

	int		multipartTotalLen = -1;
	int		multipartDataRead = 0;

	int		clientFd;
	int		errorFileFd = -1;
	int		responseFileFd = -1;
	int		pipeToCgi[2] = {-1, -1};
	int		pipeFromCgi[2] = {-1, -1};
	int		bytesSent = 0;

	int		clientTimeOutLimit = 3;

	bool	stateFlags[8] = {}; // JUST FOR DEBUG

	std::string	clientIP; // we should probably get this in the constructor
	std::string	requestString;
	std::string responseBody;
	std::string responseString;
	
	clientInfo(int clientFd, const serverInfo *server) : relatedServer(server), clientFd(clientFd)
	{
		startTime = std::chrono::high_resolution_clock::now();
	}

};