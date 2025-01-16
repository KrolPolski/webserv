#include "ConnectionHandler.hpp"


ConnectionHandler::ConnectionHandler()
{
}


ConnectionHandler::~ConnectionHandler()
{
	closeAllSockets();
}


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
			return (sigIntExit());
		
		checkClientTimeOut();

		int	readySocketCount = poll(&m_pollfdVec[0], m_pollfdVec.size(), 0);
		if (readySocketCount == -1)
		{
			if (isSigInt)
				return (sigIntExit());
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

void	ConnectionHandler::checkClientTimeOut()
{
	for (auto &client : m_clientVec)
	{
		client.curTime = std::chrono::high_resolution_clock::now();
		if (client.curTime - client.startTime >= std::chrono::seconds(client.clientTimeOutLimit))
		{
			std::cerr << RED << "\nClient disconnected due to time out\n" << RESET << "\n";
			if (client.respHandler != nullptr && client.respHandler->m_cgiHandler != nullptr)
			{
				pid_t cgiChildPid = client.respHandler->m_cgiHandler->getCgiChildPid();
				if (cgiChildPid != -1)
					kill(cgiChildPid, SIGKILL); // Or sigint...? And errorhandling?
			}	
			client.status = DISCONNECT; // WE NEED SOME KIND OF ERROR PAGE!
		}
	}
}

void	ConnectionHandler::handleClientAction(const pollfd &pollFdStuct)
{
	clientInfo *clientPTR = getClientPTR(pollFdStuct.fd);
	if (clientPTR == nullptr)
	{
		std::cerr << RED << "Client data could not be recieved; client not found\n" << RESET;
		return ;
	}

	switch (clientPTR->status)
	{

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

		case EXECUTE_CGI:
		{
			if (!clientPTR->stateFlags[EXECUTE_CGI])
			{
				clientPTR->stateFlags[EXECUTE_CGI] = true;
				std::cout << GREEN << "\nCLIENT EXECUTE_CGI!\n" << RESET;
			}

			if (clientPTR->pipeToCgi[1] == pollFdStuct.fd && pollFdStuct.revents & POLLOUT)
			{
		//		std::cout << GREEN << "\nGoing to write in ToCgi Pipe!\n" << RESET;

				if (clientPTR->respHandler->m_cgiHandler->writeToCgiPipe() == -1)
					clientPTR->status = DISCONNECT; // This is wrong, we need an error page! But what type of page...?
			}
			else if (clientPTR->pipeToCgi[0] == pollFdStuct.fd && pollFdStuct.revents & POLLIN)
				clientPTR->respHandler->m_cgiHandler->setPipeToCgiReadReady();
			else if (clientPTR->pipeFromCgi[1] == pollFdStuct.fd && pollFdStuct.revents & POLLOUT)
				clientPTR->respHandler->m_cgiHandler->setPipeFromCgiWriteReady();

			int executeStatus = clientPTR->respHandler->m_cgiHandler->executeCgi();

			if (executeStatus == 0 || executeStatus == 2 || executeStatus == -1)
			{
				removeFromPollfdVec(clientPTR->pipeToCgi[0]);
				clientPTR->pipeToCgi[0] = -1;
				removeFromPollfdVec(clientPTR->pipeToCgi[1]);
				clientPTR->pipeToCgi[1] = -1;
				removeFromPollfdVec(clientPTR->pipeFromCgi[1]);
				clientPTR->pipeFromCgi[1] = -1;

				if (executeStatus == 0)
					clientPTR->status = BUILD_CGI_RESPONSE;
				else if (executeStatus == -1)
					clientPTR->status = DISCONNECT; // This is wrong, we need an error page! But what type of page...?
			}

			break ;
		}

		case BUILD_CGI_RESPONSE:
		{
			if (!clientPTR->stateFlags[BUILD_CGI_RESPONSE])
			{
				clientPTR->stateFlags[BUILD_CGI_RESPONSE] = true;
				std::cout << GREEN << "\nCLIENT BUILD_CGI_RESPONSE!\n" << RESET;
			}

			if (clientPTR->pipeFromCgi[0] == pollFdStuct.fd && pollFdStuct.revents & POLLIN)
			{
		//		std::cout << GREEN << "\nGoing to buildCgiResponse() function!\n" << RESET;
		//		std::cout << GREEN << "\nThe pipeFromCGI fd is: " << clientPTR->pipeFromCgi[0] << RESET;

				if (clientPTR->respHandler->m_cgiHandler->buildCgiResponse(clientPTR) == -1)
					clientPTR->status = DISCONNECT; // This is wrong, we need an error page! But what type of page...?
			}
			break ;
		}

		case SEND_RESPONSE:
		{
			if (!clientPTR->stateFlags[SEND_RESPONSE])
			{
				clientPTR->stateFlags[SEND_RESPONSE] = true;
				std::cout << GREEN << "\nCLIENT SEND_RESPONSE!\n" << RESET;

			//	std::cout << RED << "ResponseStr:\n" << RESET << clientPTR->responseString << "\n";
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

			clientCleanUp(clientPTR);
			removeFromClientVec(clientPTR);

		//	std::cout << RED << "SERVER STATUS:\n" << RESET
		//	<< "Server vec size: " << m_serverVec.size() << "\n"
		//	<< "Client vec size: " << m_clientVec.size() << "\n"
		//	<< "Poll vec size: " << m_pollfdVec.size() << "\n";

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
		// std::cout << GREEN << "Creating new client\n" << RESET;
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
	clientPTR->requestString.append(buf, recievedBytes);

	if (clientPTR->reqType == UNDEFINED && checkRequestType(clientPTR) == CHUNKED)
	{
		clientPTR->reqType = CHUNKED;
		if (checkChunkedEnd(clientPTR))
			std::cout << RED << "Chunked request ended\n" << RESET;

	}
	else if (clientPTR->reqType == UNDEFINED && checkRequestType(clientPTR) == MULTIPART)
	{
		std::cout << GREEN << "It's multipart\n" << RESET;

		clientPTR->reqType = MULTIPART;
		clientPTR->multipartTotalLen = getMultidataLength(clientPTR);
		std::cout << GREEN << "The req body length is: " << clientPTR->multipartTotalLen << "\n" << RESET;

	}
	else if (clientPTR->reqType == CHUNKED && checkChunkedEnd(clientPTR))
	{
		std::cout << GREEN << "Chunked request ended\n" << RESET;
	//	std::cout << RED << "Recieved request:\n\n" << RESET << clientPTR->requestString << "\n\n";
		clientPTR->status = DISCONNECT; // TEST
	}
	else if (clientPTR->reqType == MULTIPART)
	{
		clientPTR->multipartDataRead += recievedBytes;

		if (clientPTR->multipartDataRead == clientPTR->multipartTotalLen)
		{
			std::cout << GREEN << "Multipart request ended\n" << RESET;
			std::cout << GREEN << "Multipart body data read: " << RESET << clientPTR->multipartDataRead << "\n";
			std::cout << GREEN << "Request string length: " << RESET << clientPTR->requestString.size() << "\n";

	//		std::cout << RED << "Recieved request:\n\n" << RESET << clientPTR->requestString << "\n\n";

			multipartSaveTest(clientPTR); // --> Panu's simple test function

		/*	clientPTR->status = PARSE_REQUEST;
			parseClientRequest(clientPTR);
			if (clientPTR->status == BUILD_REPONSE)
				addNewPollfd(clientPTR->responseFileFd);
			else if (clientPTR->status == EXECUTE_CGI)
			{
				addNewPollfd(clientPTR->pipeToCgi[0]);
				addNewPollfd(clientPTR->pipeToCgi[1]);
				addNewPollfd(clientPTR->pipeFromCgi[0]);
				addNewPollfd(clientPTR->pipeFromCgi[1]);
			} */
		}
	}
	else if (recievedBytes < bufLen)
	{
		std::cout << RED << "Recieved request:\n\n" << RESET << clientPTR->requestString << "\n\n";

		clientPTR->status = PARSE_REQUEST;
		parseClientRequest(clientPTR);
		if (clientPTR->status == BUILD_REPONSE)
			addNewPollfd(clientPTR->responseFileFd);
		else if (clientPTR->status == EXECUTE_CGI)
		{
			addNewPollfd(clientPTR->pipeToCgi[0]);
			addNewPollfd(clientPTR->pipeToCgi[1]);
			addNewPollfd(clientPTR->pipeFromCgi[0]);
			addNewPollfd(clientPTR->pipeFromCgi[1]);
		}

		// std::cout << RED << "Parse done" << RESET << "\n";
		// std::cout << RED << "Client status: " << clientPTR->status << RESET << "\n\n";
	}
	
}

clientRequestType	ConnectionHandler::checkRequestType(clientInfo *clientPTR)
{
	if (clientPTR->requestString.find("\r\nTransfer-Encoding: chunked\r\n") != std::string::npos)
		return CHUNKED;
	else if (clientPTR->requestString.find("\r\nContent-Type: multipart/form-data") != std::string::npos)
		return MULTIPART;
	else
		return UNDEFINED;
}

bool	ConnectionHandler::checkChunkedEnd(clientInfo *clientPTR)
{
	if (clientPTR->requestString.find("\r\n0\r\n\r\n") != std::string::npos) // Might be more efficient to look only the recieved buffer, not entire string!!
		return true;
	else
		return false;
}

int		ConnectionHandler::getMultidataLength(clientInfo *clientPTR)
{
	std::string	strToFind = "Content-Length: ";
	size_t 		startIdx = clientPTR->requestString.find(strToFind);

	if (startIdx == std::string::npos)
	{
		std::cerr << RED << "\ngetMultidataLength() failed:\n" << RESET << "Content-Length header was not found" << "\n\n";
		// What should we do then...? Code 500?
	}
	startIdx += strToFind.length(); // +1 to get over the extra space

	size_t endIdx = clientPTR->requestString.find("\r\n", startIdx);
	if (endIdx == std::string::npos)
	{
		std::cerr << RED << "\ngetMultidataLength() failed:\n" << RESET << "Content-Length header is empty" << "\n\n";
		// What should we do then...? Code 500?
	}

	std::string lengthStr = clientPTR->requestString.substr(startIdx, endIdx - startIdx);

	return (std::stoi(lengthStr));
}


void	ConnectionHandler::multipartSaveTest(clientInfo *clientPTR)
{
	std::string temp = "boundary=";
	size_t boundaryStartIdx = clientPTR->requestString.find(temp);
	boundaryStartIdx += temp.length();
	size_t boundaryEndIdx = clientPTR->requestString.find("\r\n", boundaryStartIdx);
	std::string boundaryStr = clientPTR->requestString.substr(boundaryStartIdx, boundaryEndIdx - boundaryStartIdx);


	temp = "Content-Type: image/jpeg\r\n\r\n";
	size_t binaryStartIdx = clientPTR->requestString.find(temp);
	binaryStartIdx += temp.length();
	size_t binaryEndIdx = clientPTR->requestString.find(boundaryStr + "--");
	std::string binaryStr = clientPTR->requestString.substr(binaryStartIdx, binaryEndIdx - binaryStartIdx - 2);

	std::cout << RED << "Binary data len: " << binaryStr.size() << "\n" << RESET;

	std::ofstream outFile("./home/images/uploads/image1.jpeg");

	outFile << binaryStr;

	outFile.close();

	std::cout << GREEN << "Multipart save test complete\n" << RESET;

	
	clientPTR->status = PARSE_REQUEST;
	parseClientRequest(clientPTR);
	if (clientPTR->status == BUILD_REPONSE)
		addNewPollfd(clientPTR->responseFileFd);
	else if (clientPTR->status == EXECUTE_CGI)
	{
		addNewPollfd(clientPTR->pipeToCgi[0]);
		addNewPollfd(clientPTR->pipeToCgi[1]);
		addNewPollfd(clientPTR->pipeFromCgi[0]);
		addNewPollfd(clientPTR->pipeFromCgi[1]);
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
}


/*
	SEND DATA TO CLIENT
*/

void		ConnectionHandler::sendDataToClient(clientInfo *clientPTR)
{
	// std::cout << RED << "RESPONSE:\n" << RESET << clientPTR->responseString << "\n";

	size_t	sendLenMax = 1000000;
	size_t	sendDataLen = clientPTR->responseString.size();
	if (sendDataLen > sendLenMax)
		sendDataLen = sendLenMax;

	// Send response to client
	size_t sendBytes = send(clientPTR->clientFd, clientPTR->responseString.c_str(), sendDataLen, 0);

	if (sendBytes < 0) // should 0 be included...?
	{
		std::cerr << RED << "\nsend() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Do we need any other error handling...? For example close the client connection?
		clientPTR->status = DISCONNECT; // check this!
	}

	clientPTR->bytesSent += sendBytes;
	if (sendBytes <= clientPTR->responseString.size())
		clientPTR->responseString.erase(0, sendBytes);

	// std::cout << RED << "Bytes sent:\n" << RESET << clientPTR->bytesSent << "\n";

	if (clientPTR->bytesSent == clientPTR->responseString.size() || clientPTR->responseString.size() == 0)
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

void	ConnectionHandler::removeFromPollfdVec(int &fdToRemove)
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

void	ConnectionHandler::removeClientFdsFromPollVec(clientInfo *clientPTR)
{
	if (clientPTR->clientFd != -1)
	{
		removeFromPollfdVec(clientPTR->clientFd);
		close(clientPTR->clientFd); // Should we check return value...?
	}
	if (clientPTR->errorFileFd != -1)
	{
		removeFromPollfdVec(clientPTR->errorFileFd);
		close(clientPTR->errorFileFd); // Should we check return value...?
	}
	if (clientPTR->responseFileFd != -1)
	{
		removeFromPollfdVec(clientPTR->responseFileFd);
		close(clientPTR->responseFileFd); // Should we check return value...?
	}
	if (clientPTR->pipeToCgi[0] != -1)
	{
		removeFromPollfdVec(clientPTR->pipeToCgi[0]);
		close(clientPTR->pipeToCgi[0]); // Should we check return value...?
	}
	if (clientPTR->pipeToCgi[1] != -1)
	{
		removeFromPollfdVec(clientPTR->pipeToCgi[1]);
		close(clientPTR->pipeToCgi[1]); // Should we check return value...?
	}
	if (clientPTR->pipeFromCgi[0] != -1)
	{
		removeFromPollfdVec(clientPTR->pipeFromCgi[0]);
		close(clientPTR->pipeFromCgi[0]); // Should we check return value...?
	}
	if (clientPTR->pipeFromCgi[1] != -1)
	{
		removeFromPollfdVec(clientPTR->pipeFromCgi[1]);
		close(clientPTR->pipeFromCgi[1]); // Should we check return value...?
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
		|| obj.responseFileFd == clientFd
		|| obj.pipeToCgi[0] == clientFd
		|| obj.pipeToCgi[1] == clientFd
		|| obj.pipeFromCgi[0] == clientFd
		|| obj.pipeFromCgi[1] == clientFd)
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
	{
		if (clientPTR->respHandler->m_cgiHandler != nullptr)
			delete clientPTR->respHandler->m_cgiHandler;
		delete clientPTR->respHandler;
	}
	
	removeClientFdsFromPollVec(clientPTR);
}

// Returns -1 for error handling purposes
int		ConnectionHandler::sigIntExit()
{
	std::cout << GREEN << "\nRecieved SIGINT signal, exiting program\n" << RESET;

	for (auto &obj : m_clientVec)
		clientCleanUp(&obj);

	return (-1);
}



