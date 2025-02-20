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

#include <fstream> // just for multipart data save test


class ConnectionHandler
{
	public:

	ConnectionHandler();
	~ConnectionHandler();

	int		initServers(char *configFile);
	int		startServers();

	std::vector<serverInfo> &getServerVec();
	std::vector<clientInfo> &getClientVec();

//////

	private:
	
	std::multimap<std::string, ConfigurationHandler>	m_configMap;

	std::vector<serverInfo>	m_serverVec;
	std::vector<clientInfo>	m_clientVec;
	std::vector<pollfd>		m_pollfdVec;

	// socket functions
	int		initServerSocket(const unsigned int portNum, ConfigurationHandler &config);
	unsigned int convertIP(std::string IPaddress);
	void	closeAllSockets();
	bool	checkForServerSocket(const int socketFd);

	// poll() helper functions
	void	addNewPollfd(int newFd);
	void	removeFromPollfdVec(const int &fdToRemove);
	void	removeClientFdsFromPollVec(clientInfo *clientPTR);

	// client data functions
	void	checkClientTimeOut();
	void	resetClientTimeOut(clientInfo *clientPTR);
	void	handleClientAction(const pollfd &pollFdStuct);
	void	acceptNewClient(const unsigned int serverFd);
	void	recieveDataFromClient(const unsigned int clientFd, clientInfo *clientPTR);
	void	parseClientRequest(clientInfo *clientPTR);
	void	sendDataToClient(clientInfo *clientPTR);
	void	clientCleanUp(clientInfo *clientPTR);
	bool	checkBodySize(uint contentLength, clientInfo *clientPTR);
	bool	unChunkRequest(clientInfo *clientPTR);

	// request receiveing & parsing
	int		parseRequest(clientInfo *clientPTR);
	int		getRelatedServer(clientInfo *clientPTR);
	int 	splitStartLine(clientInfo *clientPTR, requestParseInfo	&parseInfo);
	clientRequestType	checkRequestType(clientInfo *clientPTR);
	bool	checkChunkedEnd(clientInfo *clientPTR);
	int		getBodyLength(clientInfo *clientPTR);
	bool	checkForBody(clientInfo *clientPTR);

	// multipart data
	void	writeUploadData(clientInfo *clientPTR);

	// client helper functions
	clientInfo	*getClientPTR(const int clientFd);
	pollfd		*getClientPollfd(const int clientFd);
	void		removeFromClientVec(clientInfo *clientPTR);

	int	sigIntExit();


};
