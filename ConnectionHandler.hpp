#pragma once

#include <iostream>
#include <vector>
#include <cerrno> // are these ok...?
#include <cstring> // are these ok...?
#include <unistd.h> // close()
#include <fcntl.h> // fcntl() --> (for making sockets NONBLOCK)
#include <poll.h> // poll()
#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in --> for internet domain sockets
#include <memory>
#include "Types.hpp"
#include "Structs.hpp"
#include "ResponseHandler.hpp"
#include "ConfigurationHandler.hpp"


class ConnectionHandler
{
	public:

	ConnectionHandler();
	~ConnectionHandler();

	// Do we need canonical form here...?

	int		initServers(char *configFile);
	int		startServers();

//////

	private:
	
	std::map<std::string, ConfigurationHandler>	m_configMap;

	// map: ConfigHandler	m_config;

	std::vector<serverInfo>	m_serverVec;
	std::vector<clientInfo>	m_clientVec;
	std::vector<pollfd>		m_pollfdVec;

	// socket functions
	int		initServerSocket(const unsigned int portNum);
	void	closeAllSockets();
	bool	checkForServerSocket(const int socketFd);

	// poll() helper functions
	void	addNewPollfd(int newFd);
	void	removeFromPollfdVec(int fdToRemove);
	void	removeClientFdsFromPollVec(clientInfo *clientPTR);


	// client data functions
	void	checkClientTimeOut();
	void	handleClientAction(const pollfd &pollFdStuct);
	void	acceptNewClient(const unsigned int serverFd);
	void	recieveDataFromClient(const unsigned int clientFd, clientInfo *clientPTR);
	void	parseClientRequest(clientInfo *clientPTR);
	void	sendDataToClient(clientInfo *clientPTR);
	void	clientCleanUp(clientInfo *clientPTR);

	// request parsing
	int		parseRequest(clientInfo *clientPTR);

	// server helper functions

	serverInfo *getServerByFd(const int fd);

	// client helper functions
	clientInfo	*getClientPTR(const int clientFd);
	pollfd		*getClientPollfd(const int clientFd);
	void		removeFromClientVec(clientInfo *clientPTR);


/*	// These are just for testing
	std::string createHomeResponse();
	std::string createSecondPageResponse();
*/

};
