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
	bool 	requestReady;
//	bool	responseReady; --> is this needed...?
	std::string	requestString;
	std::string responseString;
	int		bytesSent;

	clientInfo(int clientFd) : 
	fd(clientFd), requestReady(false), requestString(""), responseString(""), bytesSent(0)
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

/*enum contentTypes
{
	HTML,
	CSS,
	JAVASCRIPT,
	PNG,
	JSON
}*/


/*
	GLOBAL VARIABLE
*/

extern bool	isSigInt; // is this needed...?
