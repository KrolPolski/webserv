#include "CgiHandler.hpp"
#include "Structs.hpp"

/*
	CONSTRUCTOR
*/

CgiHandler::CgiHandler(clientInfo &client) : m_client(client)
{
	m_scriptReady = false;
	m_responseReady = false;
	m_cgiTimeOut = 3;
	m_pipeFromCgi[0] = -1;
	m_pipeFromCgi[1] = -1;
	m_pipeToCgi[0] = -1;
	m_pipeToCgi[1] = -1;

	CgiTypes 	type = m_client.parsedRequest.cgiType;

	if (type == PHP)
		//m_pathToInterpreter = "/opt/homebrew/bin/php-cgi"; homebrew path is different
		m_pathToInterpreter = m_client.relatedServer->serverConfig->getCgiPath("/cgi/") + "/php-cgi";
	else if (type == PYTHON)
		m_pathToInterpreter = m_client.relatedServer->serverConfig->getCgiPath("/cgi/") + "/python3"; // check this later // with the regex changes in that merge i might change this. ----- Patrik

	std::string currentDir = std::filesystem::current_path();
	std::string root = m_client.relatedServer->serverConfig->getRoot("/");
	m_pathToScript = currentDir + "/" + root + m_client.parsedRequest.filePath;

	setExecveArgs();
	setExecveEnvArr();

}

/*
	SET EXECVE ARRAYS
*/

void	CgiHandler::setExecveArgs()
{
	m_argsForExecve[0] = (char * )m_pathToInterpreter.c_str(); // check the casting later
	m_argsForExecve[1] = (char * )m_pathToScript.c_str(); // check the casting later
	m_argsForExecve[2] = NULL;
}

void	CgiHandler::setExecveEnvArr()
{
	std::map<std::string, std::string>	&headerMap = m_client.parsedRequest.headerMap;

	m_contenLen = "CONTENT_LENGTH=";
	if (headerMap.find("Content-Length") != headerMap.end())
		m_contenLen += headerMap.at("Content-Length");
	m_contenType = "CONTENT_TYPE=";
	if (headerMap.find("Content-Type") != headerMap.end())
		m_contenType += headerMap.at("Content-Type");
	m_queryStr = "QUERY_STRING=" + m_client.parsedRequest.queryString;
	m_pathInfo = "PATH_INFO=" + m_pathToScript;
	m_requestMethod = "REQUEST_METHOD=" + m_client.parsedRequest.method;
	m_scriptFileName = "SCRIPT_FILENAME=" + m_pathToScript.substr(m_pathToScript.find_last_of('/') + 1); // is this wrong...?
	m_scriptName = "SCRIPT_NAME=" + m_client.relatedServer->serverConfig->getRoot("/") + m_client.parsedRequest.filePath;
	m_redirectStatus = "REDIRECT_STATUS=200";
	m_serverProtocol = "SERVER_PROTOCOL=HTTP/1.1";
	m_gatewayInterface = "GATEWAY_INTERFACE=CGI/1.1";
	m_remote_addr = "REMOTE_ADDR=127.0.0.1"; // change later
	m_serverName = "SERVER_NAME=local_host"; // change later
	m_serverPort = "SERVER_PORT=" + m_client.relatedServer->serverConfig->getPort(); // check this later

	// Make strings compatible with execve
	m_envArrExecve[0] = (char *) m_contenLen.c_str(); // check the casting later
	m_envArrExecve[1] = (char *) m_contenType.c_str(); // check the casting later
	m_envArrExecve[2] = (char *) m_queryStr.c_str(); // check the casting later
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
int	CgiHandler::executeCgi()
{	
	if (pipe(m_pipeFromCgi) == -1) // make these O_NONBLOCK
		return (errorExit("Pipe() failed"));
	if (pipe(m_pipeToCgi) == -1) // make these O_NONBLOCK
		return (errorExit("Pipe() failed"));

	pid_t	cgiPid;
	cgiPid = fork();
	if (cgiPid == -1)
		return (errorExit("Fork() failed"));

	if (cgiPid == 0)
		return (cgiChildProcess());
	else
	{
		// In parent/main process

		if (close(m_pipeFromCgi[1]) == -1)
			return (errorExit("Close() failed")); // is this needed...?
		if (close(m_pipeToCgi[0]) == -1)
			return (errorExit("Close() failed")); // is this needed...?

		if (m_client.parsedRequest.method == "POST")
		{
			const char *buf = m_client.parsedRequest.rawContent.c_str();
			size_t len = m_client.parsedRequest.rawContent.length();

			if (write(m_pipeToCgi[1], buf, len + 1) == -1)
				return (errorExit("Write() failed"));
			if (close(m_pipeToCgi[1]) == -1)
				return (errorExit("Close() failed")); // is this needed...?
		}
		else
		{
			if (close(m_pipeToCgi[1]) == -1)
				return (errorExit("Close() failed")); // is this needed...?
		}

		if (waitForChildProcess(cgiPid) == -1)
			return (-1);

		/*
			PANU:

			This needs a rebuild. 
			It needs to happen like the response building: read one chunk per round, and check if all has been read.
			Also: Do I need to do poll() check before entering this...?
		*/


		char 	buffer[1024];
		int		bytesRead;
		int		readPerCall = 1023;
		bytesRead = read(m_pipeFromCgi[0], buffer, readPerCall); // This needs to go thorugh poll()...?
		if (bytesRead == -1)
			return (errorExit("Read() failed")); // is this needed...?

		buffer[bytesRead] = '\0';

		std::string bufStr = buffer;
		m_responseBody += bufStr;

		/*
			NOTE:

			We need to also check in the ConnectionHandler the Cgi status somehow.
			I mean, when m_responseReady == true, only then we send the response!
		*/
		if (bytesRead < readPerCall)
		{
			buildCgiResponse();
			m_responseReady = true;
		}

		if (close(m_pipeFromCgi[0]) == -1)
			return (errorExit("Close() failed")); // is this needed...?
;
	}
	return (0);

}

/*
	CGI EXECUTE SUBFUNCTIONS
*/

int		CgiHandler::cgiChildProcess()
{
	size_t 		index = m_pathToScript.find_last_of('/');
	int			len = m_pathToScript.length() - (m_pathToScript.length() - index);
	std::string scriptDirectoryPath = m_pathToScript.substr(0, len);

	if (close (m_pipeFromCgi[0]) == -1 || close (m_pipeToCgi[1]) == -1)
		return (errorExit("Close() failed")); // is this needed...?

	if (chdir(scriptDirectoryPath.c_str()) == -1)
		return (errorExit("Chdir() failed"));

	if (dup2(m_pipeFromCgi[1], STDOUT_FILENO) == -1 || dup2(m_pipeToCgi[0], STDIN_FILENO) == -1)
		return (errorExit("Dup2() failed"));

	if (close (m_pipeFromCgi[1]) == -1 || close (m_pipeToCgi[0]) == -1)
		return (errorExit("Close() failed")); // is this needed...?


	if (execve(m_pathToInterpreter.c_str(), m_argsForExecve, m_envArrExecve) == -1)
	{
		std::cerr << RED << "\nExecve() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		return (1);
	}
	return (0);
}

int	CgiHandler::waitForChildProcess(pid_t &cgiPid)
{
	int statloc;
	int	waitpidStatus = 0;

	/*
		PANU:
		Evantually do this timeout check in the poll loop!
	*/
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::high_resolution_clock> curTime;
	while (waitpidStatus == 0)
	{
		curTime = std::chrono::high_resolution_clock::now();
		if (curTime - startTime >= std::chrono::seconds(m_cgiTimeOut))
		{
			kill(cgiPid, SIGKILL); // Or sigint...? And errorhandling?
			std::cerr << RED << "\nCGI failed due to time out\n" << RESET << "\n";
			closeAllFd();
			return (-1);
		}
		waitpidStatus = waitpid(cgiPid, &statloc, WNOHANG);
	}

	if (waitpidStatus == -1)
		return (errorExit("Waitpid() failed"));
	if (WIFEXITED(statloc) == 1)
	{
		if (WEXITSTATUS(statloc) != 0)
			return (-1);
	}
	else if (WIFSIGNALED(statloc) == 1)
	{
		if (WTERMSIG(statloc) == 2)
			std::cout << RED << "Cgi child process got interrupted by SIGINT" << "\n" << RESET;
		else if (WTERMSIG(statloc) == 3)
			std::cout << RED << "Cgi child process got interrupted by SIGQUIT" << "\n" << RESET;
		return (-1);
	}

	return (0);
}

void	CgiHandler::buildCgiResponse()
{
	if (m_client.parsedRequest.cgiType == PHP)
	{
		// Remove extra headers created by php-cgi
		size_t index = m_responseBody.find_first_of('\n');
		index = m_responseBody.find_first_of('\n', index + 1);
		m_responseBody = m_responseBody.substr(index + 1, m_responseBody.length() - index);	
	}
	
	// Should we do these in the server manually like these
	// OR do we expect the CGI scripts themselves to create headers...?
	m_responseHeaders = "HTTP/1.1 200 OK\r\n";
	m_responseHeaders += "Content-Type: text/html\r\n"; // check this
	m_responseHeaders += "Content-Length: ";
	m_responseHeaders += std::to_string(m_responseBody.length());
	m_responseHeaders += "\r\n\r\n";
	m_client.responseString = m_responseHeaders + m_responseBody;

//	std::cout << GREEN << "RESPONSE STRING:\n" << RESET << m_client.responseString << "\n\n";
}


int		CgiHandler::errorExit(std::string errStr)
{
	std::cerr << RED << "\n" << errStr << ":\n" << RESET << std::strerror(errno) << "\n\n";
	closeAllFd();
	return (-1);
}

void	CgiHandler::closeAllFd()
{
	closeAndInitFd(m_pipeFromCgi[0]);
	closeAndInitFd(m_pipeFromCgi[1]);
	closeAndInitFd(m_pipeToCgi[0]);
	closeAndInitFd(m_pipeToCgi[1]);
}

void	CgiHandler::closeAndInitFd(int fd)
{
	if (fd >= 0)
	{
		close(fd); // error handling...?
		fd = -1;
	}
}





/*
	DEBUG PRINTING:


	std::cerr << RED << "CHILD m_pathToInterpreter: \n" << RESET << m_pathToInterpreter.c_str() << "\n\n";

	std::cerr << RED << "CHILD m_argsForExecve 0: \n" << RESET << m_argsForExecve[0] << "\n";
	std::cerr << RED << "CHILD m_argsForExecve 1: \n" << RESET << m_argsForExecve[1] << "\n\n";

	std::cerr << RED << "CHILD m_envArrExecve 0: \n" << RESET << m_envArrExecve[0] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 1: \n" << RESET << m_envArrExecve[1] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 2: \n" << RESET << m_envArrExecve[2] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 3: \n" << RESET << m_envArrExecve[3] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 4: \n" << RESET << m_envArrExecve[4] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 5: \n" << RESET << m_envArrExecve[5] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 6: \n" << RESET << m_envArrExecve[6] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 7: \n" << RESET << m_envArrExecve[7] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 8: \n" << RESET << m_envArrExecve[8] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 9: \n" << RESET << m_envArrExecve[9] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 10: \n" << RESET << m_envArrExecve[10] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 11: \n" << RESET << m_envArrExecve[11] << "\n";
	std::cerr << RED << "CHILD m_envArrExecve 12: \n" << RESET << m_envArrExecve[12] << "\n";
*/