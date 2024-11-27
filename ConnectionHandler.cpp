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

}

/*
	INIT SERVERS
*/

int		ConnectionHandler::initServers(unsigned int *portArr, int portCount)
{
	for (int i = 0; i < portCount; ++i)
	{
		int socketfd = initServerSocket(portArr[i]);
		if (socketfd == -1)
			return (-1);
		
		m_serverVec.push_back({socketfd}); // here we need to add all relevant info to serverInfo struct
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

	// make socket non-blocking
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
	if (listen(socketFd, 2) == -1) // how long should the queue be...?
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
		int	readySocketCount = poll(&m_pollfdVec[0], m_pollfdVec.size(), 0);
		if (readySocketCount == -1)
		{
			std::cerr << RED << "\npoll() failed:\n" << RESET << std::strerror(errno) << "\n\n";
			// Error handling
			closeAllSockets();
			return (-1); // should we quit the server...? Probably not
		}
		else if (readySocketCount == 0)
			continue ;

		// Check status of polled sockets
		for (int i = 0; i < (int)m_pollfdVec.size(); ++i) // the int casting... not good
		{
			// Check if server has a new connection --> OWN FUNCTION...?
			if (m_pollfdVec[i].revents & POLLIN && isServerSocket(m_pollfdVec[i].fd))
				acceptNewClient(m_pollfdVec[i].fd);
			else if (m_pollfdVec[i].revents & POLLIN && !isServerSocket(m_pollfdVec[i].fd))
				recieveDataFromClient(m_pollfdVec[i].fd);
			else if (m_pollfdVec[i].revents & POLLOUT && !isServerSocket(m_pollfdVec[i].fd)) // do i need to check the response ready-flag...?
				sendDataToClient(m_pollfdVec[i].fd);
		}
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
		addNewPollfd(newClientFd);
		m_clientVec.push_back({newClientFd});
	}
}

/*
	RECIEVE DATA FROM CLIENT
*/

void	ConnectionHandler::recieveDataFromClient(const unsigned int clientFd)
{
	clientInfo *clientPTR = getClientPTR(clientFd); // nullptr check...?
	char	buf[1024] = {0};
	int		bufLen = 1023; // what is the correct size for recv() buffer...?

	int recievedBytes = recv(clientFd, buf, bufLen, 0);
	if (recievedBytes <= 0)
	{
		if (recievedBytes == 0)
			std::cout << "Connection with client has been closed\n";
		else
			std::cerr << RED << "\nrecv() failed\n" << RESET << std::strerror(errno) << "\n\n";

		removeFromClientVec(clientFd);
		removeFromPollfdVec(clientFd);
		return ;
	}

	buf[recievedBytes] = '\0';
	clientPTR->requestString += buf;

	// If we managed to recieve everything from client
	if (recievedBytes < bufLen)
	{
		clientPTR->requestReady = true;
		getClientPollfd(clientFd)->events = POLLOUT;

		/*
		RYAN:

		Here you should take the current clientInfo struct (with clientPTR)
		and parse the request that is in clientPTR->requestString
		and then form a proper response and store it in clientPTR->responseString
		*/

		// JUST A TEST FOR NOW

		if (clientPTR->requestString[5] == 's')
			clientPTR->responseString = createSecondPageResponse();
		else
			clientPTR->responseString = createHomeResponse();
	}
	
}

/*
	SEND DATA TO CLIENT
*/

void		ConnectionHandler::sendDataToClient(const unsigned int clientFd)
{
	clientInfo *clientPTR = getClientPTR(clientFd);

	// Send response to client
	int sendBytes = send(clientPTR->fd, clientPTR->responseString.c_str(), clientPTR->responseString.length(), 0);

	if (sendBytes <= 0) // should 0 be included...?
	{
		std::cerr << RED << "\nsend() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Do we need any other error handling...? For example close the client connection?
	}

	clientPTR->bytesSent += sendBytes;

	// What should we do if these don't match...? Most likely do another send starting from response[bytesSent]...?
	if (clientPTR->bytesSent == (int)clientPTR->responseString.length()) // int casting... not good
	{
		removeFromClientVec(clientPTR->fd);
		removeFromPollfdVec(clientPTR->fd);
	}
}

/*
	POLLFD VECTOR UPDATE FUNCTIONS
*/

void	ConnectionHandler::addNewPollfd(int newFd)
{
	pollfd	tempPollfd;
	tempPollfd.fd = newFd;
	tempPollfd.events = POLLIN; // should we also have POLLOUT here for every socket...?

	m_pollfdVec.push_back(tempPollfd);
}

void	ConnectionHandler::removeFromPollfdVec(int fdToRemove)
{
	for (int i = 0; i < (int)m_pollfdVec.size(); ++i)
	{
		if (m_pollfdVec[i].fd == fdToRemove)
		{
			close(fdToRemove); // do i need to check close() for fail...?
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

bool	ConnectionHandler::isServerSocket(const int fdToCheck)
{
	for (auto server : m_serverVec)
	{
		if (server.fd == fdToCheck)
			return (true);
	}
	return (false);
}

// returns nullptr if (for some reason) the client is not found
clientInfo	*ConnectionHandler::getClientPTR(const int clientFd)
{
	for (auto &obj : m_clientVec)
	{
		if (obj.fd == clientFd)
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

void		ConnectionHandler::removeFromClientVec(const int clientFd)
{
	for (int i = 0; i < (int)m_clientVec.size(); ++i)
	{
		if (m_clientVec[i].fd == clientFd)
		{
			m_clientVec.erase(m_clientVec.begin() + i); // can this be done smarter than erase()...?
			return ;
		}
	}
}

/*
	TEST FUNCTIONS TO CREATE HTML CONTENT
*/

std::string ConnectionHandler::createHomeResponse()
{
	// Create html string
	std::string	htmlString = "<html><body style=\"background-color:lightgreen;\">";
	htmlString += "<h1>Hello world! =)</h1>";
	htmlString += "<a href=\"secondPage\">Click me to go to the Second page</a>";
	htmlString += "</body></html>";

	// Create headers for HTTP response
	std::string	headers;

	headers = "HTTP/1.1 200 OK\r\n";
	headers += "Content-Type: text/html\r\n";
	headers += "Content-Length: ";
	headers += std::to_string(htmlString.length());
	headers += "\r\n\r\n";

	std::string finalResponse = headers + htmlString;

	return (finalResponse);
}

std::string ConnectionHandler::createSecondPageResponse()
{
	// Create html string
	std::string	htmlString = "<html><body style=\"background-color:lightblue;\">";
	htmlString += "<h1>Oh nice, we have another page!</h1>";
	htmlString += "<a href=\"/\">Go back to home page</a>";
	htmlString += "</body></html>";

	// Create headers for HTTP response
	std::string	headers;

	headers = "HTTP/1.1 200 OK\r\n";
	headers += "Content-Type: text/html\r\n";
	headers += "Content-Length: ";
	headers += std::to_string(htmlString.length());
	headers += "\r\n\r\n";

	std::string finalResponse = headers + htmlString;

	return (finalResponse);
}
