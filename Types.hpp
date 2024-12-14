#pragma once

/*
	A headerfile for storing custom data-types and defines that are needed in multiple classes
*/

#include <iostream>

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
	STRUCTS
*/

#include <map> // is this needed here...?

struct requestParseInfo
{
	bool 	isCgi = false;

	std::string		startLine;
	std::map<std::string, std::string>	headerMap;
	std::string		rawContent;

	std::string	method;
	std::string requestedFilePath;
	std::string queryString;
};

// Not final version
struct clientInfo
{
//	serverInfo	&server;
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
	
	clientInfo(int clientFd) : fd(clientFd)
	{
	}
};

// Not final version
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

enum requestTypes
{
	GET,
	POST,
	DELETE,
	INVALID
};

enum CgiTypes
{
	PHP,
	PYTHON,
	NONE
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
	SIGNAL HANDLING
*/

#include <signal.h> // for signal handling

extern bool	isSigInt;

void	sigIntHandler(int signal);
int		sigIntMessage();
