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
	}
	catch(const std::exception& e)
	{
		webservLog.webservLog(ERROR, e.what(), true);
		return -1;
	}
	try
	{
		extractServerBlocks(m_configMap, rawFile);
	}
	catch(const std::exception& e)
	{
		webservLog.webservLog(ERROR, e.what(), true);
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
			webservLog.webservLog(ERROR, "Error: Port is invalid", true); // start servers that have valid ports, stoi fails get discarded and user gets informed.
			return -1;
		}
		
		int socketfd = initServerSocket(portNum, iter->second);
		
		if (socketfd != -1)
		{
			m_serverVec.push_back({socketfd, &iter->second});
			addNewPollfd(socketfd);
		}
	}
	if (m_configMap.size() == 0)
		return -1;
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
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "socket() failed:" + errorString, true);
		return (-1);
	}

	// This solves the bind() issue
	int opt = 1;
	if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "setsockopt() failed:" + errorString, true);
		close(socketFd);
		return (-1);
	}

	// make socket non-blocking
	if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "fcntl() failed:" + errorString, true);
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
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "bind() failed:" + errorString, true);
		close(socketFd);
		return (-1);
	}

	// make socket ready to accept connections with listen()
	if (listen(socketFd, 20) == -1)
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "listen() failed:" + errorString, true);
		close(socketFd);
		return (-1);
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
				std::string errorString = std::strerror(errno);
				webservLog.webservLog(ERROR, "poll() failed:" + errorString, true);
        for (auto &obj : m_clientVec)
					clientCleanUp(&obj);
				for (auto &server : m_serverVec)
					close(server.fd);
				return (-1);
			}
		}
		else if (readySocketCount == 0)
			continue ;

		// Check status of polled sockets
		for (int i = 0; i < (int)m_pollfdVec.size(); ++i)
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
			webservLog.webservLog(ERROR, "Client disconnected due to time out", true);
			if (client.respHandler != nullptr && client.respHandler->m_cgiHandler != nullptr)
			{
				pid_t cgiChildPid = client.respHandler->m_cgiHandler->getCgiChildPid();
				if (cgiChildPid != -1)
					kill(cgiChildPid, SIGKILL);
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
		webservLog.webservLog(ERROR, "Client data could not be recieved; client not found for FD: " + std::to_string(pollFdStuct.fd), true);
    removeFromPollfdVec(pollFdStuct.fd);
		close(pollFdStuct.fd);
		return ;
	}

	switch (clientPTR->status)
	{
		case RECIEVE_REQUEST:
		{
			if (!clientPTR->stateFlags[RECIEVE_REQUEST])
			{
				clientPTR->stateFlags[RECIEVE_REQUEST] = true;
				webservLog.webservLog(INFO, "CLIENT RECIEVE_REQUEST!", false);
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
				webservLog.webservLog(INFO, "CLIENT SAVE_FILE!", false);
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
				webservLog.webservLog(INFO, "CLIENT BUILD_ERRORPAGE!", false);
			}

			if (clientPTR->errorFileFd == pollFdStuct.fd && pollFdStuct.revents & POLLIN)
			{
				clientPTR->respHandler->buildErrorResponse(clientPTR);
				resetClientTimeOut(clientPTR);
			}
			break ;
		}

		case BUILD_RESPONSE:
		{
			if (!clientPTR->stateFlags[BUILD_RESPONSE])
			{
				clientPTR->stateFlags[BUILD_RESPONSE] = true;
				webservLog.webservLog(INFO, "CLIENT BUILD_RESPONSE!", false);
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
				webservLog.webservLog(INFO, "CLIENT EXECUTE_CGI!", false);
			}

			// Write the request body to CGI script & check that pipe FDs are ready
			if (clientPTR->bytesToWriteInCgi != 0 && clientPTR->pipeToCgi[1] == pollFdStuct.fd && pollFdStuct.revents & POLLOUT)
			{
				if (clientPTR->respHandler->m_cgiHandler->writeToCgiPipe(clientPTR) == -1)
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

			// Execute CGI child process
			int executeStatus = clientPTR->respHandler->m_cgiHandler->executeCgi(clientPTR, m_serverVec, m_clientVec);

			// Wait for the CGI child process to finish && read the output from child process.
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
				{
					if (clientPTR->respHandler->m_cgiHandler->finishCgiResponse(clientPTR) == -1)
					{
						clientPTR->respHandler->setResponseCode(500);
						clientPTR->respHandler->openErrorResponseFile(clientPTR);
						addNewPollfd(clientPTR->errorFileFd);
						return ;
					}
					resetClientTimeOut(clientPTR);
				}
				else if (executeStatus == 2 && clientPTR->pipeFromCgi[0] == pollFdStuct.fd && pollFdStuct.revents & POLLIN)
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
				else if (executeStatus == -1)
				{
					clientPTR->respHandler->setResponseCode(500);
					clientPTR->respHandler->openErrorResponseFile(clientPTR);
					addNewPollfd(clientPTR->errorFileFd);
					return ;
				}
			}

			break ;
		}

		case BUILD_CGI_RESPONSE:
		{
			if (!clientPTR->stateFlags[BUILD_CGI_RESPONSE])
			{
				clientPTR->stateFlags[BUILD_CGI_RESPONSE] = true;
				webservLog.webservLog(INFO, "CLIENT BUILD_CGI_RESPONSE!", false);
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
				webservLog.webservLog(INFO, "CLIENT SEND_RESPONSE!", false);
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
				webservLog.webservLog(INFO, "CLIENT DISCONNECT!", false);
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
	int newClientFd = accept(serverFd, nullptr, nullptr);
	if (newClientFd == -1)
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "accept() failed: " + errorString, false);
		return ;
	}
	else
	{
		if (fcntl(newClientFd, F_SETFL, O_NONBLOCK) == -1)
		{
      close (newClientFd);
			std::string errorString = std::strerror(errno);
			webservLog.webservLog(ERROR, "fcntl() failed: " + errorString, true);
			return ;
		}
		addNewPollfd(newClientFd);
		m_clientVec.push_back({newClientFd});
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
	static std::string conLenStr = "Content-Length: ";
	if (header.empty())
	{
		header = request.substr(0, bodyIndex - 2);
	}
	try
	{
		webservLog.webservLog(INFO, "Getting body", false);
		static std::string	unChunked;
		body = request.substr(bodyIndex, request.size() - bodyIndex);
		webservLog.webservLog(INFO, "Entering while loop", false);
		while (1)
		{
			if (body.find("\r\n", startIndex) == body.npos)
			{
				webservLog.webservLog(INFO, "Exiting, waiting for next chunk hex size", false);
				break ;
			}
			size_t indexAfterHex = body.find("\r\n", startIndex);
			std::string	hexValue = body.substr(startIndex, indexAfterHex - startIndex);
			uint bytesToRead = std::stoi(hexValue, nullptr, 16);
			if (body.size() - startIndex < bytesToRead + 2)
			{
				webservLog.webservLog(INFO, "Chunk body not complete, waiting for more data", false);
				return true;
			}
			contentLength += bytesToRead;
			if (checkBodySize(contentLength, clientPTR) == false)
				return false;
			if (bytesToRead == 0)
			{
				webservLog.webservLog(INFO, "Finished chunked reading request", false);
				header += conLenStr;
				break ;
			}

			startIndex = indexAfterHex + 2;
			unChunked += body.substr(startIndex, bytesToRead);
			startIndex += bytesToRead + 2;

		}
		webservLog.webservLog(INFO, "Exiting while loop", false);
		if (std::search(header.begin(), header.end(), conLenStr.begin(), conLenStr.end()) != header.end()) // here we need to get check for correct thing
		{
			header += std::to_string(contentLength) + "\r\n\r\n";
			clientPTR->requestString.erase();
			clientPTR->requestString += header + unChunked;
			webservLog.webservLog(INFO, "Full request unchunked\n" + clientPTR->requestString, false);
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
	}
	return true;
}

void	ConnectionHandler::recieveDataFromClient(const unsigned int clientFd, clientInfo *clientPTR)
{
	char	buf[100024] = {0};
	int		bufLen = 100023;

	int recievedBytes = recv(clientFd, buf, bufLen, 0);
	if (recievedBytes <= 0)
	{
		if (recievedBytes == 0)
		{
			webservLog.webservLog(ERROR, "Connection has been closed by the client during recv() operation\n", true);
			clientPTR->status = DISCONNECT;
		}
		else
		{
			std::string errorString = std::strerror(errno);
			webservLog.webservLog(ERROR, "recv() failed " + errorString, true);
			clientPTR->respHandler->setResponseCode(500);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			addNewPollfd(clientPTR->errorFileFd);
		}
		return ;
	}

	buf[recievedBytes] = '\0';
	clientPTR->requestString.append(buf, recievedBytes);

	if (clientPTR->reqType == UNDEFINED)
	{
		clientPTR->reqType = checkRequestType(clientPTR);

		// if we have more than 8KB characters of only headers, we say it's a bad request
		if (clientPTR->reqType == UNDEFINED && clientPTR->requestString.size() > 8192)
		{
			webservLog.webservLog(ERROR, "Bad request: Request headers too long", true);
			clientPTR->respHandler->setResponseCode(400);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			addNewPollfd(clientPTR->errorFileFd);
			return ;
		}

		size_t headerEndIdx = clientPTR->requestString.find("\r\n\r\n");
		if (headerEndIdx != std::string::npos && headerEndIdx > 8192)
		{
			webservLog.webservLog(ERROR, "Bad request: Request headers too long", true);
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
			clientPTR->respHandler->setResponseCode(413);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			addNewPollfd(clientPTR->errorFileFd);
			return ;
		}
		if (checkChunkedEnd(clientPTR))
		{		
			clientPTR->status = PARSE_REQUEST;
			parseClientRequest(clientPTR);
		}

	}
	else if (clientPTR->reqType == MULTIPART)
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
		if (checkBodySize(clientPTR->reqBodyDataRead, clientPTR) == false)
		{
			clientPTR->respHandler->setResponseCode(413);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			addNewPollfd(clientPTR->errorFileFd);
			return ;
		}

		if (clientPTR->bodyOK || clientPTR->reqBodyDataRead == clientPTR->reqBodyLen)
		{

			clientPTR->status = PARSE_REQUEST;
			parseClientRequest(clientPTR);
		}

	}
	else if (clientPTR->reqType == OTHER)
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
		if (checkBodySize(clientPTR->reqBodyDataRead, clientPTR) == false)
		{
			clientPTR->respHandler->setResponseCode(413);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			addNewPollfd(clientPTR->errorFileFd);
			return ;
		}

		if (clientPTR->bodyOK || clientPTR->reqBodyDataRead == clientPTR->reqBodyLen)
		{
			clientPTR->status = PARSE_REQUEST;
			parseClientRequest(clientPTR);
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
	uint bodySize = body.size();
	if (checkBodySize(bodySize, clientPTR) == false)
	{
		clientPTR->respHandler->setResponseCode(413);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		addNewPollfd(clientPTR->errorFileFd);
		return false;
	}

	if (bodySize == clientPTR->reqBodyLen)
		return true;
	else
		return false;

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
	if (clientPTR->chunkedOK == true)
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
		webservLog.webservLog(ERROR, "getBodyLength() failed: Content-Length header was not found", true);
		clientPTR->respHandler->setResponseCode(400);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		addNewPollfd(clientPTR->errorFileFd);
		return -1;
	}
	startIdx += strToFind.length();

	size_t endIdx = clientPTR->requestString.find("\r\n", startIdx);
	if (endIdx == std::string::npos)
	{
		webservLog.webservLog(ERROR, "getBodyLength() failed: HTTP header format error", true);
		clientPTR->respHandler->setResponseCode(400);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		addNewPollfd(clientPTR->errorFileFd);
		return -1;
	}
	else if (startIdx == endIdx)
	{
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
		addNewPollfd(clientPTR->errorFileFd);
		return ;
	}

	clientPTR->respHandler->setRequestType(clientPTR);
	if (clientPTR->status == BUILD_ERRORPAGE)
	{
		addNewPollfd(clientPTR->errorFileFd);
		return ;
	}

	clientPTR->respHandler->handleRequest(clientPTR);
	if (clientPTR->status == BUILD_ERRORPAGE)
	{
		addNewPollfd(clientPTR->errorFileFd);
		return ;
	}

	if (clientPTR->status == BUILD_RESPONSE)
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

void	ConnectionHandler::writeUploadData(clientInfo *clientPTR)
{
	size_t binaryStartIdx = clientPTR->multipartFileDataStartIdx;
	size_t binaryEndIdx = clientPTR->requestString.find(clientPTR->multipartBoundaryStr + "--");
	if (binaryEndIdx == std::string::npos)
	{
		std::cerr << RED << "writeUploadData() failed: " << RESET << "request is missing ending boundary" << "\n\n";
		clientPTR->respHandler->setResponseCode(400);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		addNewPollfd(clientPTR->errorFileFd);
		return ;
	}

	std::string binaryStr = clientPTR->requestString.substr(binaryStartIdx, binaryEndIdx - binaryStartIdx);

	if (write(clientPTR->uploadFileFd, binaryStr.c_str(), binaryStr.size()) == -1)
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "write() in file upload failed " + errorString, true);
		clientPTR->respHandler->setResponseCode(500); // check this later
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		addNewPollfd(clientPTR->errorFileFd);
		return ;
	}
	else // successful upload
	{
		clientPTR->respHandler->setResponseCode(201);
		clientPTR->respHandler->build201Response(clientPTR, clientPTR->uploadWebPath);
	}

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
	size_t	sendLenMax = 1000000;
	size_t	sendDataLen = clientPTR->responseString.size();
	if (sendDataLen > sendLenMax)
		sendDataLen = sendLenMax;

	// Send response to client
	int sendBytes = send(clientPTR->clientFd, clientPTR->responseString.c_str(), sendDataLen, 0);

	if (sendBytes <= 0)
	{
		if (sendBytes == 0)
		{
			webservLog.webservLog(INFO, "Connection closed by the client during send() operation", false);
			clientPTR->status = DISCONNECT;
		}
		else
		{	
			std::string errorString = std::strerror(errno);
			webservLog.webservLog(ERROR, "send() failed: " + errorString, true);
			clientPTR->respHandler->setResponseCode(500);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			addNewPollfd(clientPTR->errorFileFd);
		}
		return ;
	}

	clientPTR->bytesSent += sendBytes;
	if (sendBytes <= (int)clientPTR->responseString.size())
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

void	ConnectionHandler::removeFromPollfdVec(const int &fdToRemove)
{
	if (fdToRemove == -1)
		return ;

	for (int i = 0; i < (int)m_pollfdVec.size(); ++i)
	{
		if (m_pollfdVec[i].fd == fdToRemove)
		{
			m_pollfdVec.erase(m_pollfdVec.begin() + i);
			return ;
		}
	}
}

void	ConnectionHandler::removeClientFdsFromPollVec(clientInfo *clientPTR)
{
	if (clientPTR->clientFd != -1)
	{
		removeFromPollfdVec(clientPTR->clientFd);
		close(clientPTR->clientFd);
	}
	if (clientPTR->errorFileFd != -1)
	{
		removeFromPollfdVec(clientPTR->errorFileFd);
		close(clientPTR->errorFileFd);
	}
	if (clientPTR->responseFileFd != -1)
	{
		removeFromPollfdVec(clientPTR->responseFileFd);
		close(clientPTR->responseFileFd);
	}
	if (clientPTR->uploadFileFd != -1)
	{
		removeFromPollfdVec(clientPTR->uploadFileFd);
		close(clientPTR->uploadFileFd);
	}
	if (clientPTR->pipeToCgi[0] != -1)
	{
		removeFromPollfdVec(clientPTR->pipeToCgi[0]);
		close(clientPTR->pipeToCgi[0]);
	}
	if (clientPTR->pipeToCgi[1] != -1)
	{
		removeFromPollfdVec(clientPTR->pipeToCgi[1]);
		close(clientPTR->pipeToCgi[1]);
	}
	if (clientPTR->pipeFromCgi[0] != -1)
	{
		removeFromPollfdVec(clientPTR->pipeFromCgi[0]);
		close(clientPTR->pipeFromCgi[0]);
	}
	if (clientPTR->pipeFromCgi[1] != -1)
	{
		removeFromPollfdVec(clientPTR->pipeFromCgi[1]);
		close(clientPTR->pipeFromCgi[1]);
	}

}

/*
	CLOSE SOCKETS
*/

void	ConnectionHandler::closeAllSockets()
{
	for (auto &obj : m_pollfdVec)
	{
		close(obj.fd);
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
			m_clientVec.erase(m_clientVec.begin() + i);
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
				kill(cgiChildPid, SIGKILL);
			delete clientPTR->respHandler->m_cgiHandler;
		}
		delete clientPTR->respHandler;
	}
	removeClientFdsFromPollVec(clientPTR);
}

// Returns -1 for error handling purposes
int		ConnectionHandler::sigIntExit()
{
	webservLog.webservLog(INFO, "Recieved SIGINT signal, exiting program", true);

	for (auto &obj : m_clientVec)
		clientCleanUp(&obj);

	for (auto &server : m_serverVec)
		close(server.fd);

	return (-1);
}


std::vector<serverInfo> &ConnectionHandler::getServerVec()
{
	return m_serverVec;
}
std::vector<clientInfo> &ConnectionHandler::getClientVec()
{
	return m_clientVec;
}



