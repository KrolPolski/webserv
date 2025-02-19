#include "CgiHandler.hpp"
#include "Logger.hpp"
#include "Structs.hpp"

CgiHandler::CgiHandler(clientInfo *clientPTR)
{
	m_pipeToCgiWriteDone = false;
	m_pipeToCgiReadReady = false;
	m_pipeFromCgiWriteReady = false;
	m_childProcRunning = false;
	m_scriptReady = false;
	m_responseReady = false;

	m_childProcPid = -1;

	CgiTypes 	type = clientPTR->parsedRequest.cgiType;

	if (type == PHP)
		m_pathToInterpreter = clientPTR->relatedServer->serverConfig->getCgiPathPHP(clientPTR->parsedRequest.filePath) + "/php-cgi";
	else if (type == PYTHON)
		m_pathToInterpreter = clientPTR->relatedServer->serverConfig->getCgiPathPython(clientPTR->parsedRequest.filePath) + "/python3";

	std::string currentDir = std::filesystem::current_path();
	std::string root = clientPTR->relatedServer->serverConfig->getRoot("/");
	m_pathToScript = currentDir + "/" + root + clientPTR->parsedRequest.filePath;

	setExecveArgs();
	setExecveEnvArr(clientPTR);

}

void	CgiHandler::setExecveArgs()
{
	m_argsForExecve[0] = (char * )m_pathToInterpreter.c_str();
	m_argsForExecve[1] = (char * )m_pathToScript.c_str();
	m_argsForExecve[2] = NULL;
}

void	CgiHandler::setExecveEnvArr(clientInfo *clientPTR)
{
	std::map<std::string, std::string>	&headerMap = clientPTR->parsedRequest.headerMap;

	m_contenLen = "CONTENT_LENGTH=";
	if (headerMap.find("Content-Length") != headerMap.end())
		m_contenLen += headerMap.at("Content-Length");
	m_contenType = "CONTENT_TYPE=";
	if (headerMap.find("Content-Type") != headerMap.end())
		m_contenType += headerMap.at("Content-Type");
	m_queryStr = "QUERY_STRING=" + clientPTR->parsedRequest.queryString;
	m_pathInfo = "PATH_INFO=" + m_pathToScript;
	m_requestMethod = "REQUEST_METHOD=" + clientPTR->parsedRequest.method;
	m_scriptFileName = "SCRIPT_FILENAME=" + m_pathToScript.substr(m_pathToScript.find_last_of('/') + 1);
	m_scriptName = "SCRIPT_NAME=" + clientPTR->relatedServer->serverConfig->getRoot("/") + clientPTR->parsedRequest.filePath;
	m_redirectStatus = "REDIRECT_STATUS=200";
	m_serverProtocol = "SERVER_PROTOCOL=HTTP/1.1";
	m_gatewayInterface = "GATEWAY_INTERFACE=CGI/1.1";
	m_remote_addr = "REMOTE_ADDR=" + clientPTR->relatedServer->serverConfig->getHost();
	m_serverName = "SERVER_NAME=" + clientPTR->relatedServer->serverConfig->getNames();
	m_serverPort = "SERVER_PORT=" + clientPTR->relatedServer->serverConfig->getPort();

	// Make strings compatible with execve
	m_envArrExecve[0] = (char *) m_contenLen.c_str();
	m_envArrExecve[1] = (char *) m_contenType.c_str();
	m_envArrExecve[2] = (char *) m_queryStr.c_str();
	m_envArrExecve[3] = (char *) m_serverProtocol.c_str();
	m_envArrExecve[4] = (char *) m_gatewayInterface.c_str();
	m_envArrExecve[5] = (char *) m_pathInfo.c_str();
	m_envArrExecve[6] = (char *) m_requestMethod.c_str();
	m_envArrExecve[7] = (char *) m_scriptFileName.c_str();
	m_envArrExecve[8] = (char *) m_redirectStatus.c_str();
	m_envArrExecve[9] = (char *) m_scriptName.c_str();
	m_envArrExecve[10] = (char *) m_remote_addr.c_str();
	m_envArrExecve[11] = (char *) m_serverName.c_str();
	m_envArrExecve[12] = (char *) m_serverPort.c_str();
	m_envArrExecve[13] = NULL;

}

/*
	EXECUTE CGI
*/

// Returns -1 on failure
int	CgiHandler::executeCgi(clientInfo *clientPTR, std::vector<serverInfo> &serverVec, std::vector<clientInfo> &clientVec)
{	
	if (!m_pipeToCgiReadReady || !m_pipeFromCgiWriteReady)
		return (1);

	if (m_childProcRunning == false)
	{
		m_childProcRunning = true;

		m_childProcPid = fork();
		if (m_childProcPid == -1)
			return (errorExit(clientPTR, "Fork() failed: ", false));

		if (m_childProcPid == 0)
		{
			return (cgiChildProcess(clientPTR, serverVec, clientVec));
		}
		else
		{
			// In parent/main process

			close(clientPTR->pipeFromCgi[1]);
			close(clientPTR->pipeToCgi[0]);

			if (clientPTR->bytesToWriteInCgi == 0 && clientPTR->pipeToCgi[1] != -1)
				close(clientPTR->pipeToCgi[1]);

			return (checkWaitStatus(clientPTR));
		}
	}
	else
	{
		if (clientPTR->bytesToWriteInCgi == 0 && clientPTR->pipeToCgi[1] != -1) 
			close(clientPTR->pipeToCgi[1]);
		return (checkWaitStatus(clientPTR));
	}
}

int		CgiHandler::writeToCgiPipe(clientInfo *clientPTR)
{
	if (m_pipeToCgiWriteDone)
		return (0);

	if (clientPTR->parsedRequest.method == "POST")
	{
		if (clientPTR->bytesToWriteInCgi == -1)
			clientPTR->bytesToWriteInCgi = clientPTR->reqBodyLen;

		int bytesWritten = write(clientPTR->pipeToCgi[1], clientPTR->parsedRequest.rawContent.c_str(), clientPTR->bytesToWriteInCgi);
			
		if (bytesWritten == -1)
			return (errorExit(clientPTR, "Write() in CGI failed: ", false));

		clientPTR->bytesToWriteInCgi -= bytesWritten;
		clientPTR->parsedRequest.rawContent.erase(0, bytesWritten);

		
	}
	else if (clientPTR->parsedRequest.method == "GET")
	{
		clientPTR->respHandler->m_cgiHandler->setPipeToCgiReadReady();
		clientPTR->bytesToWriteInCgi = 0;
	}
	return (0);
}

/*
	CGI EXECUTE SUBFUNCTIONS
*/

// Closing other client related FDs and server FDs. They are not needed by CGI child process.
void	CgiHandler::closeExtraFD(clientInfo *clientPTR, std::vector<serverInfo> &serverVec, std::vector<clientInfo> &clientVec)
{
	for (auto &client : clientVec)
	{
		if (client.clientFd != clientPTR->clientFd)
			closeAndDeleteClient(&client);
		else
			close(client.clientFd);
	}

	for (auto server : serverVec)
		close(server.fd);
}

int		CgiHandler::cgiChildProcess(clientInfo *clientPTR, std::vector<serverInfo> serverVec, std::vector<clientInfo> clientVec)
{
	size_t 		index = m_pathToScript.find_last_of('/');
	int			len = m_pathToScript.length() - (m_pathToScript.length() - index);
	std::string scriptDirectoryPath = m_pathToScript.substr(0, len);

	if (close(clientPTR->pipeFromCgi[0]) == -1 || close(clientPTR->pipeToCgi[1]) == -1)
		return (errorExit(clientPTR, "Close() failed in CGI child process: ", true));

	if (chdir(scriptDirectoryPath.c_str()) == -1)
		return (errorExit(clientPTR, "Chdir() failed in CGI child process: ", true));

	if (dup2(clientPTR->pipeFromCgi[1], STDOUT_FILENO) == -1 || dup2(clientPTR->pipeToCgi[0], STDIN_FILENO) == -1)
		return (errorExit(clientPTR, "Dup2() failed in CGI child process: ", true));

	if (close(clientPTR->pipeFromCgi[1]) == -1 || close(clientPTR->pipeToCgi[0]) == -1)
		return (errorExit(clientPTR, "Close() failed in CGI child process: ", true));

	webservLog.closeLogFileStream();
	closeExtraFD(clientPTR, serverVec, clientVec);

	if (execve(m_pathToInterpreter.c_str(), m_argsForExecve, m_envArrExecve) == -1)
	{
		std::cerr << RED << "\nExecve() failed in CGI child process:\n" << RESET << std::strerror(errno) << "\n\n"; // this can't user Logger!!
		closeAndDeleteClient(clientPTR);
		return (-1);
	}

	return (0);
}

int	CgiHandler::checkWaitStatus(clientInfo *clientPTR)
{
	int statloc;
	int	waitpidStatus;

	waitpidStatus = waitpid(m_childProcPid, &statloc, WNOHANG);
	if (waitpidStatus == 0)
		return (2);

	m_childProcPid = -1; // if waitpid != 0, the child process has ended. For cleanup purposes we mark the PID as -1.

	if (waitpidStatus == -1)
		return (errorExit(clientPTR, "Waitpid() failed: ", false));
	if (WIFEXITED(statloc) == 1)
	{
		if (WEXITSTATUS(statloc) != 0)
			return (-1);
	}
	else if (WIFSIGNALED(statloc) == 1)
	{
		if (WTERMSIG(statloc) == 2)
			webservLog.webservLog(ERROR, "Cgi child process got interrupted by SIGINT", true);
		else if (WTERMSIG(statloc) == 3)
			webservLog.webservLog(ERROR, "Cgi child process got interrupted by SIGQUIT", true);
		return (-1);
	}

	return (0);
}

int CgiHandler::getCgiResponseBodyLen()
{
	size_t bodyStartIdx = m_responseBody.find("\r\n\r\n");
	if (bodyStartIdx == std::string::npos)
	{
		webservLog.webservLog(ERROR, "CGI script has wrong response format", true);
		return -1;
	}
	bodyStartIdx += 4;

	return (m_responseBody.size() - bodyStartIdx);
}

int CgiHandler::finishCgiResponse(clientInfo *clientPTR)
{
	m_responseHeaders = "HTTP/1.1 200 OK\r\n";
	m_responseHeaders += "Server: " + clientPTR->relatedServer->serverConfig->getNames() + "\r\n"; // CHECK THIS!!
	if (m_responseBody.find("Content-Length: ") == std::string::npos)
	{
		m_responseHeaders += "Content-Length: ";
		int contentLen = getCgiResponseBodyLen();
		if (contentLen == -1)
			return -1;
		m_responseHeaders += std::to_string(contentLen);
		m_responseHeaders += "\r\n";
	}
	if (m_responseBody.find("Content-Type: ") == std::string::npos)
		m_responseHeaders += "Content-Type: text/html\r\n"; // either this or just plain text...?
	clientPTR->responseString = m_responseHeaders + m_responseBody;
	clientPTR->status = SEND_RESPONSE;

	close(clientPTR->pipeFromCgi[0]); // Do we need to check close() return value...?

	return (0);
}

int	CgiHandler::buildCgiResponse(clientInfo *clientPTR)
{
	char 	buffer[10001]; // should this be bigger...?
	int		bytesRead;
	int		readPerCall = 10000; // should this be bigger...?

	bytesRead = read(clientPTR->pipeFromCgi[0], buffer, readPerCall);
	if (bytesRead == -1)
		return (errorExit(clientPTR, "Read() failed: ", false));

	buffer[bytesRead] = '\0';

	std::string bufStr = buffer;
	m_responseBody += bufStr;
	clientPTR->bytesReceivedFromCgi += bytesRead;

	return (0);
}


int		CgiHandler::errorExit(clientInfo *clientPTR, std::string errStr, bool isChildProc)
{
	std::string errMessage = std::strerror(errno);

	webservLog.webservLog(ERROR, errStr + errMessage, true);

	if (isChildProc)
		closeAndDeleteClient(clientPTR);
	return (-1);
}

void	CgiHandler::closeAndDeleteClient(clientInfo *clientPTR)
{
	closeAndInitFd(clientPTR->clientFd);
	closeAndInitFd(clientPTR->responseFileFd);
	closeAndInitFd(clientPTR->errorFileFd);
	closeAndInitFd(clientPTR->uploadFileFd);
	closeAndInitFd(clientPTR->pipeFromCgi[0]);
	closeAndInitFd(clientPTR->pipeFromCgi[1]);
	closeAndInitFd(clientPTR->pipeToCgi[0]);
	closeAndInitFd(clientPTR->pipeToCgi[1]);
	if (clientPTR->respHandler != nullptr)
	{
		if (clientPTR->respHandler->m_cgiHandler != nullptr)
			delete clientPTR->respHandler->m_cgiHandler;
		delete clientPTR->respHandler;
	}
}

void	CgiHandler::closeAndInitFd(int &fd)
{
	if (fd >= 0)
	{
		close(fd); // error handling...?
		fd = -1;
	}
}

void CgiHandler::setPipeToCgiReadReady(void)
{
	m_pipeToCgiReadReady = true;
}

void CgiHandler::setPipeFromCgiWriteReady(void)
{
	m_pipeFromCgiWriteReady = true;
}

pid_t &CgiHandler::getCgiChildPid()
{
	return (m_childProcPid);
}

