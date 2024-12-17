#pragma once

/*
	A headerfile for storing custom data-types and defines that are needed in multiple classes
*/

#include <iostream>
#include "ConfigurationHandler.hpp"

/*
	DEFINES (some nice colors for terminal output =))
*/

# define RED 	"\033[31m"
# define GREEN	"\033[32m"
# define RESET	"\033[0m"
// # define BLUE	"\033[34m"
// # define PURPLE	"\033[35m"
// # define CYAN   "\033[36m"


/*
	ENUMS	
*/


enum requestTypes
{
	GET,
	POST,
	DELETE,
	INVALID
};

enum CgiTypes
{
	NONE,
	PHP,
	PYTHON
};

/*enum contentTypes
{
	HTML,
	CSS,
	JAVASCRIPT,
	PNG,
	JSON
}*/

/*
	STRUCTS
*/

#include <map> // is this needed here...?

struct serverInfo
{
	int				fd;
	unsigned int	port;
	std::string		name;

	// need to update this later
	serverInfo(int serverFd, unsigned int serverPort, std::string serverName) : 
	fd(serverFd), port(serverPort), name(serverName)
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
};



/*
	SIGNAL HANDLING
*/

#include <signal.h> // for signal handling

extern bool	isSigInt;

void	sigIntHandler(int signal);
int		sigIntMessage();
