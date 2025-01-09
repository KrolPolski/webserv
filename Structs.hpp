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

	clientStatus	status = CONNECTED;

	int		fd;
	int		bytesSent = 0;

	bool 	requestReady = false;
//	bool	responseReady; --> is this needed...?

	std::string	clientIP; // we should probably get this in the constructor
	std::string	requestString;
	std::string responseHeaders; // might not be needed
	std::string responseBody; // might not be needed
	std::string responseString;
	
	clientInfo(int clientFd, const serverInfo *server) : relatedServer(server), fd(clientFd)
	{
	}

	~clientInfo()
	{
		if (respHandler != nullptr)
			delete respHandler;
	}
};