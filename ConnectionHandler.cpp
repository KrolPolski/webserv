#include "ConnectionHandler.hpp"

/*
	CONSTRUCTOR
*/

ConnectionHandler::ConnectionHandler()
{
}


/*
	DESTRUCTOR
*/

ConnectionHandler::~ConnectionHandler()
{
	closeAllSockets();
}

/*
	INIT SERVERS
*/


int		ConnectionHandler::initServers(char *configFile)
{
	std::vector<std::string>					rawFile;

	try
	{
		readFile(fileNameCheck(configFile), rawFile);
		extractServerBlocks(m_configMap, rawFile);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return -1;
	}

	for (auto iter = m_configMap.begin(); iter != m_configMap.end(); iter++)
	{
		int	portNum;
		try
		{
			portNum = std::stoi(iter->first);
		}
		catch(const std::exception& e)
		{
			std::cerr << "Error: Port is invalid" << '\n';
			return -1;
		}
		
		int socketfd = initServerSocket(portNum);
		if (socketfd == -1)
			return (-1);
		
		m_serverVec.push_back({socketfd, &iter->second});
		addNewPollfd(socketfd);
	}

	return (0);
}

/*
	SOCKET INITIALIZATION
*/

int		ConnectionHandler::initServerSocket(const unsigned int portNum)
{
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (socketFd == -1)
	{
		std::cerr << RED << "\nsocket() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Error handling...?
		return (-1);
	}

	// This solves the bind() issue
	int opt = 1;
	if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << RED << "\nsetsockopt() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Error handling...?
		close(socketFd);
		return (-1);
	}

	// make socket non-blocking, CHECK THAT THIS IS ACTUALLY WORKING!
	if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << RED << "\nfcntl() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Error handling...?
		close(socketFd);
		return (-1);
	}

	// Create address & bind() socket to a certain port
	sockaddr_in	serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNum);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (bind(socketFd, (sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
	{
		std::cerr << RED << "\nbind() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Error handling...?
		close(socketFd);
		return (-1);
	}

	// make socket ready to accept connections with listen()
	if (listen(socketFd, 2) == -1) // how long should the queue be AND do I need to poll() before listen()...?
	{
		std::cerr << RED << "\nlisten() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Error handling...?
		close(socketFd);
		return (1);
	}

	return (socketFd);
}

/*
	START SERVERS
*/

int		ConnectionHandler::startServers()
{
	while (1)
	{
		if (isSigInt)
			return (sigIntMessage());

		int	readySocketCount = poll(&m_pollfdVec[0], m_pollfdVec.size(), 0);
		if (readySocketCount == -1)
		{
			if (isSigInt)
				return (sigIntMessage());
			else
			{
				std::cerr << RED << "\npoll() failed:\n" << RESET << std::strerror(errno) << "\n\n";
				// Error handling
				return (-1); // should we quit the server...? Probably yea
			}
		}
		else if (readySocketCount == 0)
			continue ;

		// Check status of polled sockets
		for (int i = 0; i < (int)m_pollfdVec.size(); ++i) // the int casting... not good
		{
			bool isServerSocket = checkForServerSocket(m_pollfdVec[i].fd);
			if (m_pollfdVec[i].revents & POLLIN && isServerSocket)
				acceptNewClient(m_pollfdVec[i].fd);
			else if (!isServerSocket)
				handleClientAction(m_pollfdVec[i]);
		}
	}
}

void	ConnectionHandler::handleClientAction(const pollfd &pollFdStuct)
{
	clientInfo *clientPTR = getClientPTR(pollFdStuct.fd);
	if (clientPTR == nullptr)
	{
		std::cout << RED << "Client data could not be recieved; client not found\n" << RESET;
		return ;
	}

	switch (clientPTR->status)
	{
		case CONNECTED: // This might not be needed, we could basically go straight to RECIEVE_REQUEST
		{
			if (!clientPTR->stateFlags[CONNECTED])
			{
				clientPTR->stateFlags[CONNECTED] = true;
				std::cout << GREEN << "\nCLIENT CONNECTED!\n" << RESET;
			}

			if (clientPTR->clientFd == pollFdStuct.fd)
				clientPTR->status = RECIEVE_REQUEST;
			break ;
		}
		
		case RECIEVE_REQUEST:
		{
			if (!clientPTR->stateFlags[RECIEVE_REQUEST])
			{
				clientPTR->stateFlags[RECIEVE_REQUEST] = true;
				std::cout << GREEN << "\nCLIENT RECIEVE_REQUEST!\n" << RESET;
			}

			if (clientPTR->clientFd == pollFdStuct.fd && pollFdStuct.revents & POLLIN)
				recieveDataFromClient(pollFdStuct.fd, clientPTR);
			break ;
		}

		case BUILD_ERRORPAGE:
		{
			if (!clientPTR->stateFlags[BUILD_ERRORPAGE])
			{
				clientPTR->stateFlags[BUILD_ERRORPAGE] = true;
				std::cout << GREEN << "\nCLIENT BUILD_ERRORPAGE!\n" << RESET;
			}

			if (clientPTR->errorFileFd == pollFdStuct.fd && pollFdStuct.revents & POLLIN)
				clientPTR->respHandler->buildErrorResponse(clientPTR);
			break ;
		}

		case BUILD_REPONSE:
		{
			if (!clientPTR->stateFlags[BUILD_REPONSE])
			{
				clientPTR->stateFlags[BUILD_REPONSE] = true;
				std::cout << GREEN << "\nCLIENT BUILD_REPONSE!\n" << RESET;
			}

			if (clientPTR->responseFileFd == pollFdStuct.fd && pollFdStuct.revents & POLLIN)
				clientPTR->respHandler->buildResponse(clientPTR);
			break ;
		}

		case SEND_RESPONSE:
		{
			if (!clientPTR->stateFlags[SEND_RESPONSE])
			{
				clientPTR->stateFlags[SEND_RESPONSE] = true;
				std::cout << GREEN << "\nCLIENT SEND_RESPONSE!\n" << RESET;
			}

			if (clientPTR->clientFd == pollFdStuct.fd && pollFdStuct.revents & POLLOUT)
				sendDataToClient(clientPTR);
			break ;
		}

		case DISCONNECT:
		{
			if (!clientPTR->stateFlags[DISCONNECT])
			{
				clientPTR->stateFlags[DISCONNECT] = true;
				std::cout << GREEN << "\nCLIENT DISCONNECT!\n" << RESET;
			}

			if (clientPTR->clientFd != -1)
				removeFromPollfdVec(clientPTR->clientFd);
			if (clientPTR->errorFileFd != -1)
				removeFromPollfdVec(clientPTR->errorFileFd);
			if (clientPTR->responseFileFd != -1)
				removeFromPollfdVec(clientPTR->responseFileFd);
			clientCleanUp(clientPTR);
			removeFromClientVec(clientPTR);

			/*
				NOTE!

				There appears to be some random "ghost clients" from time to time...
				don't really know why.
				They connect but don't send a request.
				This will automatically be solved once I implement the
				timeout-system for clients, but still... weird!
			*/

			break ;
		}

		default:
			break ;

	}

}

/*
	ACCEPT NEW CLIENT
*/

void		ConnectionHandler::acceptNewClient(const unsigned int serverFd)
{
	int newClientFd = accept(serverFd, nullptr, nullptr); // why are these nullptr...?	
	if (newClientFd == -1)
	{
		std::cerr << RED << "\naccept() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Error handling...?
	}
	else
	{
		if (fcntl(newClientFd, F_SETFL, O_NONBLOCK) == -1)
		{
			std::cerr << RED << "\nfcntl() failed:\n" << RESET << std::strerror(errno) << "\n\n";
			// Error handling...?
			close (newClientFd);
			return ; // What should happen here...?
		}

		serverInfo *relatedServerPTR = getServerByFd(serverFd);
		if (relatedServerPTR == nullptr)
		{
			std::cerr << RED << "\nacceptNewClient() failed:\n" << RESET << "server not found" << "\n\n";
			return ;
		}
		addNewPollfd(newClientFd);
		std::cout << GREEN << "Creating new client\n" << RESET;
		m_clientVec.push_back({newClientFd, relatedServerPTR});
	}

//	std::cout << GREEN << "NEW CLIENT Accepted with FD: " << newClientFd << "\n" << RESET;
//	std::cout << GREEN << "client vec size: " << m_clientVec.size() << "\n" << RESET;


}

/*
	RECIEVE DATA FROM CLIENT
*/

void	ConnectionHandler::recieveDataFromClient(const unsigned int clientFd, clientInfo *clientPTR)
{
	char	buf[1024] = {0};
	int		bufLen = 1023; // what is the correct size for recv() buffer...?

	int recievedBytes = recv(clientFd, buf, bufLen, 0);
	if (recievedBytes <= 0)
	{
		if (recievedBytes == 0)
			std::cout << "Connection with client has been closed\n";
		else
			std::cerr << RED << "\nrecv() failed\n" << RESET << std::strerror(errno) << "\n\n";

		clientPTR->status = DISCONNECT; // not good, we need to build some sort of response too!
		return ;
	}

	buf[recievedBytes] = '\0';
	clientPTR->requestString += buf;

	// If we managed to recieve everything from client
	if (recievedBytes < bufLen)
	{
		std::cout << RED << "Recieved request:\n\n" << RESET << clientPTR->requestString << "\n\n";

		clientPTR->status = PARSE_REQUEST;
		parseClientRequest(clientPTR);
		if (clientPTR->status == BUILD_REPONSE)
			addNewPollfd(clientPTR->responseFileFd);
		
		std::cout << RED << "Parse done" << RESET << "\n";
		std::cout << RED << "Client status: " << clientPTR->status << RESET << "\n\n";


	}
	
}

void	ConnectionHandler::parseClientRequest(clientInfo *clientPTR)
{
	parseRequest(clientPTR); // this code is in separate file ('requestParsing.cpp')

	clientPTR->respHandler = new ResponseHandler; // get's deleted in the clintInfo destructor
	clientPTR->respHandler->checkRequestType(clientPTR, clientPTR->requestString);

	if (clientPTR->status == BUILD_ERRORPAGE)
		addNewPollfd(clientPTR->errorFileFd);
	else
	{
		clientPTR->respHandler->parseRequest(clientPTR, clientPTR->requestString);
		if (clientPTR->status == BUILD_ERRORPAGE)
			addNewPollfd(clientPTR->errorFileFd);
	}
	//might have an error here now too.
}


/*
	SEND DATA TO CLIENT
*/

void		ConnectionHandler::sendDataToClient(clientInfo *clientPTR)
{
	// Send response to client
	int sendBytes = send(clientPTR->clientFd, clientPTR->responseString.c_str(), clientPTR->responseString.length(), 0);

	if (sendBytes < 0) // should 0 be included...?
	{
		std::cerr << RED << "\nsend() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Do we need any other error handling...? For example close the client connection?
	}

	clientPTR->bytesSent += sendBytes;

	std::cout << RED << "Bytes sent:\n" << RESET << clientPTR->bytesSent << "\n";

	// What should we do if these don't match...? Most likely do another send starting from response[bytesSent]...?
	if (clientPTR->bytesSent == (int)clientPTR->responseString.length()) // int casting... not good
		clientPTR->status = DISCONNECT;
}

/*
	POLLFD VECTOR UPDATE FUNCTIONS
*/

void	ConnectionHandler::addNewPollfd(int newFd)
{
	pollfd	tempPollfd;
	tempPollfd.fd = newFd;
	tempPollfd.events = POLLIN | POLLOUT;
	tempPollfd.revents = 0;

	m_pollfdVec.push_back(tempPollfd);
}

void	ConnectionHandler::removeFromPollfdVec(int fdToRemove)
{
	for (int i = 0; i < (int)m_pollfdVec.size(); ++i)
	{
		if (m_pollfdVec[i].fd == fdToRemove)
		{
			m_pollfdVec.erase(m_pollfdVec.begin() + i); // can this be done smarter than with erase()...?
			return ;
		}
	}
}

/*
	CLOSE SOCKETS
*/

// All open sockets need to be polled, so we can use the m_pollfdVec for this... right?
// Or should we store all open sockets some other way? Or should we loop throught the info structs (server & client)?
void	ConnectionHandler::closeAllSockets()
{
	for (auto &obj : m_pollfdVec)
	{
		close(obj.fd); // Possible error handling...?
	}
}

/*
	HELPER FUNCTIONS
*/

bool	ConnectionHandler::checkForServerSocket(const int fdToCheck)
{
	for (auto &server : m_serverVec)
	{
		if (server.fd == fdToCheck)
			return (true);
	}
	return (false);
}

// Returns nullptr if server is not found
serverInfo *ConnectionHandler::getServerByFd(const int fd)
{
	for (auto &server : m_serverVec)
	{
		if (server.fd == fd)
			return (&server);
	}
	return (nullptr);
}

// returns nullptr if (for some reason) the client is not found
clientInfo	*ConnectionHandler::getClientPTR(const int clientFd)
{
	for (auto &obj : m_clientVec)
	{
		if (obj.clientFd == clientFd
		|| obj.errorFileFd == clientFd
		|| obj.responseFileFd == clientFd)
			return (&obj);
	}

	return (nullptr);
}

pollfd		*ConnectionHandler::getClientPollfd(const int clientFd)
{
	for (auto &obj : m_pollfdVec)
	{
		if (obj.fd == clientFd)
			return (&obj);
	}

	return (nullptr);
}

void		ConnectionHandler::removeFromClientVec(clientInfo *clientPTR)
{
	for (int i = 0; i < (int)m_clientVec.size(); ++i)
	{
		if (&m_clientVec[i] == clientPTR)
		{
			m_clientVec.erase(m_clientVec.begin() + i); // can this be done smarter than erase()...?
			return ;
		}
	}
}

void	ConnectionHandler::clientCleanUp(clientInfo *clientPTR)
{
	if (clientPTR->respHandler != nullptr)
		delete clientPTR->respHandler;
	if (clientPTR->clientFd != -1)
		close(clientPTR->clientFd); // do i need to check close() for fail...?
	if (clientPTR->errorFileFd != -1)
		close(clientPTR->errorFileFd); // do we need error handling for close()...?
	if (clientPTR->responseFileFd != -1)
		close(clientPTR->responseFileFd); // do i need to check close() for fail...?
}



