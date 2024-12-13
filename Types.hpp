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

// Not final version
struct clientInfo
{
	int		fd;
	int		bytesSent;
	bool 	requestReady;
//	bool	responseReady; --> is this needed...?
	std::string	requestString;
	std::string responseHeaders; // might not be needed
	std::string responseBody; // might not be needed
	std::string responseString;
	

	clientInfo(int clientFd) : 
	fd(clientFd), bytesSent(0), requestReady(false), requestString(""),
	responseHeaders(""), responseBody(""), responseString("")
	{
	}
};

// Not final version
struct serverInfo
{
	int			fd;
	std::string	name;
	// something to store valid methods
	// error page information
	// root folder info

	// need to update this later
	serverInfo(int serverFd) : fd(serverFd), name("test_server_" + std::to_string(serverFd))
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
