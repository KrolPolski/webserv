#include "ConnectionHandler.hpp"
#include "Logger.hpp"

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
			std::cerr << "Error: Port is invalid" << '\n'; // start servers that have valid ports, stoi fails get discarded and user gets informed.
			return -1;
		}
		
		int socketfd = initServerSocket(portNum, iter->second);
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

int		ConnectionHandler::initServerSocket(const unsigned int portNum, ConfigurationHandler &config)
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
	serverAddress.sin_addr.s_addr = convertIP(config.getHost());


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

unsigned int ConnectionHandler::convertIP(std::string IPaddress)
{
	std::string ipBlockStr;
	int ipBlockNum = 0;
	unsigned int result = 0;
	size_t startIdx = 0;
	size_t endIdx = 0;
	std::vector<int> blockVec;

	for (int i = 0; i < 4; ++i)
	{
		endIdx = IPaddress.find('.', startIdx);

		if (endIdx == std::string::npos)
			ipBlockStr = IPaddress.substr(startIdx);
		else
			ipBlockStr = IPaddress.substr(startIdx, endIdx - startIdx);

		ipBlockNum = std::stoi(ipBlockStr);
		blockVec.push_back(ipBlockNum);
		startIdx = endIdx + 1;
	}

	for (int i = 3; i >= 0; --i)
	{
		ipBlockNum = blockVec[i];

		for (int j = 128; j > 0; (j >>= 1))
		{
			if ((ipBlockNum & j) != 0)
				result = (result | j);
		}

		if (i != 0)
			result <<= 8;
	}

	return result;
	
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
			client.respHandler->setResponseCode(408);
			client.respHandler->openErrorResponseFile(getClientPTR(client.clientFd));
			addNewPollfd(client.errorFileFd);
			client.startTime = client.curTime;
		}
	}
}

// If client has had a recent action in the server side process, we restart their time out clock
void	ConnectionHandler::resetClientTimeOut(clientInfo *clientPTR)
{
	clientPTR->startTime = std::chrono::high_resolution_clock::now();
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
			{
				recieveDataFromClient(pollFdStuct.fd, clientPTR);
				resetClientTimeOut(clientPTR);
			}
			break ;
		}

		case SAVE_FILE:
		{
			if (!clientPTR->stateFlags[SAVE_FILE])
			{
				clientPTR->stateFlags[SAVE_FILE] = true;
				std::cout << GREEN << "\nCLIENT SAVE_FILE!\n" << RESET;
			}

			if (clientPTR->uploadFileFd == pollFdStuct.fd && pollFdStuct.revents & POLLOUT)
			{
				writeUploadData(clientPTR);
				resetClientTimeOut(clientPTR);
			}
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
			{
				clientPTR->respHandler->buildErrorResponse(clientPTR);
				resetClientTimeOut(clientPTR);
			}
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
			{
				clientPTR->respHandler->buildResponse(clientPTR);
				resetClientTimeOut(clientPTR);
			}
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
				if (clientPTR->respHandler->m_cgiHandler->writeToCgiPipe() == -1)
				{
					clientPTR->respHandler->setResponseCode(500);
					clientPTR->respHandler->openErrorResponseFile(clientPTR);
					addNewPollfd(clientPTR->errorFileFd);
					return ;
				}
				resetClientTimeOut(clientPTR);
			}
			else if (clientPTR->pipeToCgi[0] == pollFdStuct.fd && pollFdStuct.revents & POLLIN)
				clientPTR->respHandler->m_cgiHandler->setPipeToCgiReadReady();
			else if (clientPTR->pipeFromCgi[1] == pollFdStuct.fd && pollFdStuct.revents & POLLOUT)
				clientPTR->respHandler->m_cgiHandler->setPipeFromCgiWriteReady();

			int executeStatus = clientPTR->respHandler->m_cgiHandler->executeCgi();

			// 0 = CGI child process successful, 2 = still waiting for child process, -1 = error
			if (executeStatus == 0 || executeStatus == 2 || executeStatus == -1)
			{
				removeFromPollfdVec(clientPTR->pipeToCgi[0]);
				clientPTR->pipeToCgi[0] = -1;
				removeFromPollfdVec(clientPTR->pipeFromCgi[1]);
				clientPTR->pipeFromCgi[1] = -1;

				if (clientPTR->bytesToWriteInCgi == 0)
				{
					removeFromPollfdVec(clientPTR->pipeToCgi[1]);
					clientPTR->pipeToCgi[1] = -1;
				}

				if (executeStatus == 0)
					clientPTR->status = BUILD_CGI_RESPONSE;
				else if (executeStatus == -1)
				{
					clientPTR->respHandler->setResponseCode(500);
					clientPTR->respHandler->openErrorResponseFile(clientPTR);
					addNewPollfd(clientPTR->errorFileFd);
					return ;
				}

				if (executeStatus != 2)
					resetClientTimeOut(clientPTR);
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
				if (clientPTR->respHandler->m_cgiHandler->buildCgiResponse(clientPTR) == -1)
				{
					clientPTR->respHandler->setResponseCode(500);
					clientPTR->respHandler->openErrorResponseFile(clientPTR);
					addNewPollfd(clientPTR->errorFileFd);
					return ;
				}
				resetClientTimeOut(clientPTR);
			}
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
			{
				sendDataToClient(clientPTR);
				resetClientTimeOut(clientPTR);
			}
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
		return ;
	}
	else
	{
		if (fcntl(newClientFd, F_SETFL, O_NONBLOCK) == -1)
		{
			std::cerr << RED << "\nfcntl() in acceptNewClient() failed:\n" << RESET << std::strerror(errno) << "\n\n";
			// Error handling...?
			close (newClientFd);
			return ;
		}

		serverInfo *relatedServerPTR = getServerByFd(serverFd);
		if (relatedServerPTR == nullptr)
		{
			std::cerr << RED << "\nacceptNewClient() failed:\n" << RESET << "server not found" << "\n\n";
			close (newClientFd);
			return ;
		}
		addNewPollfd(newClientFd);
		m_clientVec.push_back({newClientFd, relatedServerPTR});
	}


}

/*
	RECIEVE DATA FROM CLIENT
*/

bool	ConnectionHandler::checkBodySize(uint contentLength, clientInfo *clientPTR)
{
	if (contentLength > clientPTR->relatedServer->serverConfig->getMCBSize())
	{
		webservLog.webservLog(ERROR, "Max client body size reached", true);
		return false;
	}
	else
		return true;
}

bool	ConnectionHandler::unChunkRequest(clientInfo *clientPTR)
{
	webservLog.webservLog(INFO, "Unchunking", true);

	static std::string	header = "";
	static std::string	body = "";
	static uint			contentLength = 0;
	std::string			request = clientPTR->requestString;
	static size_t		bodyIndex = 0;
	static size_t		startIndex = 0;
	size_t				indexStart = 0;
	size_t				indexEnd = 0;

	indexStart = request.find("Transfer-Encoding: ");
	indexEnd = request.find("\r\n", indexStart) + 2;
	request.erase(indexStart , indexEnd - indexStart);
	bodyIndex = request.find("\r\n\r\n") + 4;
	static std::string conLenStr = "Content-Lenght: ";
	if (header.empty())
	{
		header = request.substr(0, bodyIndex - 2);
	}
	try
	{
		webservLog.webservLog(DEBUG, "Getting body", true);
		static std::string	unChunked;
		body = request.substr(bodyIndex, request.size() - bodyIndex);
		webservLog.webservLog(DEBUG, "Entering while loop", true);
		while (1)
		{
			if (body.find("\r\n", startIndex) == body.npos)
			{
				webservLog.webservLog(DEBUG, "Exiting, waiting for next chunk hex size", true);
				break ;
			}
			size_t indexAfterHex = body.find("\r\n", startIndex);
			std::string	hexValue = body.substr(startIndex, indexAfterHex - startIndex);
			uint bytesToRead = std::stoi(hexValue, nullptr, 16);
			if (body.size() - startIndex < bytesToRead + 2)
			{
				webservLog.webservLog(DEBUG, "Chunk body not complete, waiting for more data", true);
				return true;
			}
			contentLength += bytesToRead;
			if (checkBodySize(contentLength, clientPTR) == false)
				return false;
			if (bytesToRead == 0)
			{
				webservLog.webservLog(INFO, "Finished chunked reading request", true);
				header += conLenStr;
				break ;
			}

			startIndex = indexAfterHex + 2;
			unChunked += body.substr(startIndex, bytesToRead);
			startIndex += bytesToRead + 2;

		}
		webservLog.webservLog(DEBUG, "Exiting while loop", true);
		// webservLog.webservLog(DEBUG, "Reporting size", true);
		// std::cout << contentLength << std::endl;
		if (std::search(header.begin(), header.end(), conLenStr.begin(), conLenStr.end()) != header.end()) // here we need to get check for correct thing
		{
			header += std::to_string(contentLength) + "\r\n\r\n";
			std::cout << "Header\n" << header << std::endl; // info
			std::cout << "Body\n" << unChunked << std::endl; // info
			clientPTR->requestString.erase();
			clientPTR->requestString += header + unChunked;
			header.erase();
			body.erase();
			unChunked.erase();
			contentLength = 0;
			bodyIndex = 0;
			startIndex = 0;
			clientPTR->chunkedOK = true;
			return true;
		}
	}
	catch(const std::exception& e)
	{
		webservLog.webservLog(ERROR, "Unchunking chunked request failed", true);
		return false;
		// Bad request if chunk size doesnt match the chunk body
	}
	return true;
} // test with a body that is biiiiiig. change the regex for config mac client body=====Patrik

void	ConnectionHandler::recieveDataFromClient(const unsigned int clientFd, clientInfo *clientPTR)
{
	char	buf[1024] = {0};
	int		bufLen = 1023; // what is the correct size for recv() buffer...?

	int recievedBytes = recv(clientFd, buf, bufLen, 0);
	if (recievedBytes <= 0)
	{
		if (recievedBytes == 0)
		{
			std::cout << "Connection has been closed by the client during recv() operation\n";
			clientPTR->status = DISCONNECT;
		}
		else
		{
			std::cerr << RED << "\nrecv() failed\n" << RESET << std::strerror(errno) << "\n\n";
			clientPTR->respHandler->setResponseCode(500);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			addNewPollfd(clientPTR->errorFileFd);
		}
		return ;
	}

	buf[recievedBytes] = '\0';
	// CHUNKED: Is there a possibility that we get some chunked data on the first read...?
	clientPTR->requestString.append(buf, recievedBytes);

	if (clientPTR->reqType == UNDEFINED)
	{
		clientPTR->reqType = checkRequestType(clientPTR);

		// if we have more than 10000 characters of only headers, we say it's a bad request
		if (clientPTR->reqType == UNDEFINED && clientPTR->requestString.size() > 10000)
		{
			std::cerr << RED << "\nBad request:\n" << RESET << "Request headers too long" << "\n\n";
			clientPTR->respHandler->setResponseCode(400); 
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			addNewPollfd(clientPTR->errorFileFd);
			return ;
		}
	}

	if (clientPTR->reqType == CHUNKED)
	{
		if (unChunkRequest(clientPTR) == false)
		{
			clientPTR->respHandler->setResponseCode(400);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			addNewPollfd(clientPTR->errorFileFd);
			return ;
		}
		if (checkChunkedEnd(clientPTR)) // Should we unchunk as we go...? So that we can more reliable check for max_content_len?
		{		
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

	}
	else if (clientPTR->reqType == MULTIPART) // Can this be same as OTHER...?
	{

		if (clientPTR->reqBodyLen == -1)
		{
			clientPTR->reqBodyLen = getBodyLength(clientPTR);
			if (clientPTR->reqBodyLen < 0)
				return ;
			clientPTR->bodyOK = checkForBody(clientPTR);
			if (!clientPTR->bodyOK)
				return ;
		}

		if (!clientPTR->bodyOK)
			clientPTR->reqBodyDataRead += recievedBytes;

		if (clientPTR->bodyOK || clientPTR->reqBodyDataRead == clientPTR->reqBodyLen)
		{

			clientPTR->status = PARSE_REQUEST;
			parseClientRequest(clientPTR);
			if (clientPTR->status == BUILD_REPONSE)
				addNewPollfd(clientPTR->responseFileFd);
			else if (clientPTR->status == SAVE_FILE)
				addNewPollfd(clientPTR->uploadFileFd);
			else if (clientPTR->status == EXECUTE_CGI)
			{
				addNewPollfd(clientPTR->pipeToCgi[0]);
				addNewPollfd(clientPTR->pipeToCgi[1]);
				addNewPollfd(clientPTR->pipeFromCgi[0]);
				addNewPollfd(clientPTR->pipeFromCgi[1]);
			} 
		}

	}
	else if (clientPTR->reqType == OTHER) // Can this be same as MULTIPART...?
	{

		if (clientPTR->reqBodyLen == -1)
		{
			clientPTR->reqBodyLen = getBodyLength(clientPTR);
			if (clientPTR->reqBodyLen < 0)
				return ;
			clientPTR->bodyOK = checkForBody(clientPTR);
			if (!clientPTR->bodyOK)
				return ;
		}

		if (!clientPTR->bodyOK)
			clientPTR->reqBodyDataRead += recievedBytes;


		if (clientPTR->bodyOK || clientPTR->reqBodyDataRead == clientPTR->reqBodyLen)
		{
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

	}
	
}

bool	ConnectionHandler::checkForBody(clientInfo *clientPTR)
{
	if (clientPTR->requestString.find("GET") == 0 || clientPTR->requestString.find("DELETE") == 0)
		return true;

	size_t bodyStartIdx = clientPTR->requestString.find("\r\n\r\n");
	if (bodyStartIdx == std::string::npos)
		return false;

	bodyStartIdx += 4;
	std::string body = clientPTR->requestString.substr(bodyStartIdx); // TEMP! Bad idea
	int bodySize = body.size();
	if (bodySize == clientPTR->reqBodyLen)
		return true;
	else
		return false;

	/*
		Should I also check here that the received body is teh same length as the Content-Length header...?
	*/
}

clientRequestType	ConnectionHandler::checkRequestType(clientInfo *clientPTR)
{
	if (clientPTR->requestString.find("\r\nTransfer-Encoding: chunked\r\n") != std::string::npos)
		return CHUNKED;
	else if (clientPTR->requestString.find("\r\nContent-Type: multipart/form-data") != std::string::npos)
		return MULTIPART;
	else if (clientPTR->requestString.find("\r\n\r\n") != std::string::npos)
		return OTHER;
	else
		return UNDEFINED;
}

bool	ConnectionHandler::checkChunkedEnd(clientInfo *clientPTR)
{
	if (clientPTR->chunkedOK == true) // Might be more efficient to look only the recieved buffer, not entire string!!
		return true;
	else
		return false;
}

int		ConnectionHandler::getBodyLength(clientInfo *clientPTR)
{
	if (clientPTR->requestString.find("GET") == 0 || clientPTR->requestString.find("DELETE") == 0)
		return 0;

	std::string	strToFind = "Content-Length: ";
	size_t 		startIdx = clientPTR->requestString.find(strToFind);

	if (startIdx == std::string::npos)
	{
		std::cerr << RED << "\ngetBodyLength() failed:\n" << RESET << "Content-Length header was not found" << "\n\n";
		clientPTR->respHandler->setResponseCode(400); // is this ok?
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		addNewPollfd(clientPTR->errorFileFd);
		return -1;
	}
	startIdx += strToFind.length();

	size_t endIdx = clientPTR->requestString.find("\r\n", startIdx);
	if (endIdx == std::string::npos)
	{
		std::cerr << RED << "\ngetBodyLength() failed:\n" << RESET << "HTTP header format error" << "\n\n";
		clientPTR->respHandler->setResponseCode(400); // is this ok?
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		addNewPollfd(clientPTR->errorFileFd);
		return -1;
	}
	else if (startIdx == endIdx)
	{
		std::cerr << RED << "\ngetBodyLength() failed:\n" << RESET << "Content-Length header is empty" << "\n\n";
		clientPTR->respHandler->setResponseCode(400); // is this ok?
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		addNewPollfd(clientPTR->errorFileFd);
		return -1;
	}

	std::string lengthStr = clientPTR->requestString.substr(startIdx, endIdx - startIdx);

	return (std::stoi(lengthStr));
}



void	ConnectionHandler::parseClientRequest(clientInfo *clientPTR)
{

	if (parseRequest(clientPTR) == -1) // this code is in separate file ('requestParsing.cpp')
	{
		clientPTR->respHandler->setResponseCode(400);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		addNewPollfd(clientPTR->errorFileFd);
		return ;
	}

	clientPTR->respHandler->setRequestType(clientPTR);

	if (clientPTR->status == BUILD_ERRORPAGE)
		addNewPollfd(clientPTR->errorFileFd);
	else
	{
		clientPTR->respHandler->handleRequest(clientPTR);
		if (clientPTR->status == BUILD_ERRORPAGE)
			addNewPollfd(clientPTR->errorFileFd);
	}
}

void	ConnectionHandler::writeUploadData(clientInfo *clientPTR)
{
	size_t binaryStartIdx = clientPTR->multipartFileDataStartIdx;
	size_t binaryEndIdx = clientPTR->requestString.find(clientPTR->multipartBoundaryStr + "--");
	std::string binaryStr = clientPTR->requestString.substr(binaryStartIdx, binaryEndIdx - binaryStartIdx); // There was a -2, do I need it...?

	if (write(clientPTR->uploadFileFd, binaryStr.c_str(), binaryStr.size()) == -1) // Is it ok to do the write in one call...?
	{
		std::cerr << RED << "\nwrite() in file upload failed\n" << RESET << std::strerror(errno) << "\n\n";
		clientPTR->respHandler->setResponseCode(500); // check this later
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		addNewPollfd(clientPTR->errorFileFd);
		return ;
	}

	clientPTR->respHandler->openResponseFile(clientPTR, "home/images/uploadSuccessful.html"); // hard coded for now
	if (clientPTR->status == BUILD_ERRORPAGE)
		addNewPollfd(clientPTR->errorFileFd);
	else
		addNewPollfd(clientPTR->responseFileFd);

}

/*
	SEND DATA TO CLIENT
*/

void		ConnectionHandler::sendDataToClient(clientInfo *clientPTR)
{
	size_t	sendLenMax = 1000000; // Check this
	size_t	sendDataLen = clientPTR->responseString.size();
	if (sendDataLen > sendLenMax)
		sendDataLen = sendLenMax;

	std::cout << "RESPONSE:\n" << clientPTR->responseString << "\n";

	// Send response to client
	int sendBytes = send(clientPTR->clientFd, clientPTR->responseString.c_str(), sendDataLen, 0);

	if (sendBytes <= 0)
	{
		if (sendBytes == 0)
		{
			std::cout << "Connection closed by the client during send() operation\n";
			clientPTR->status = DISCONNECT;
		}
		else
		{
			std::cerr << RED << "\nsend() failed\n" << RESET << std::strerror(errno) << "\n\n";
			clientPTR->respHandler->setResponseCode(500);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			addNewPollfd(clientPTR->errorFileFd);
		}
		return ;
	}

	clientPTR->bytesSent += sendBytes;
	if (sendBytes <= (int)clientPTR->responseString.size()) // check int cast!
		clientPTR->responseString.erase(0, sendBytes);

	if (clientPTR->responseString.size() == 0)
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
	if (fdToRemove == -1)
		return ;

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
	if (clientPTR->uploadFileFd != -1) // Should we close this immidiately after the file data has been saved...?
	{
		removeFromPollfdVec(clientPTR->uploadFileFd);
		close(clientPTR->uploadFileFd); // Should we check return value...?
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
		|| obj.pipeFromCgi[1] == clientFd
		|| obj.uploadFileFd == clientFd)
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
		{
			pid_t cgiChildPid = clientPTR->respHandler->m_cgiHandler->getCgiChildPid();
			if (cgiChildPid != -1)
				kill(cgiChildPid, SIGKILL); // Or sigint...? And errorhandling?
			delete clientPTR->respHandler->m_cgiHandler;
		}
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



