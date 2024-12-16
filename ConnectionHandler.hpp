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
#include "ResponseHandler.hpp"

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

	// client data functions
	void		acceptNewClient(const unsigned int serverFd);
	void		recieveDataFromClient(const unsigned int clientFd);
	void		sendDataToClient(const unsigned int clientFd);

	// client helper functions
	clientInfo	*getClientPTR(const int clientFd);
	pollfd		*getClientPollfd(const int clientFd);
	void		removeFromClientVec(const int clientFd);


	// These are just for testing
	std::string createHomeResponse();
	std::string createSecondPageResponse();

};