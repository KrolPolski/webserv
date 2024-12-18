#include "CgiHandler.hpp"

/*
	CONSTRUCTOR
*/

CgiHandler::CgiHandler(clientInfo &client) : m_client(client)
{
	m_scriptReady = false;
	m_responseReady = false;
	m_cgiTimeOut = 3;

	CgiTypes 	type = m_client.parsedRequest.cgiType;

	if (type == PHP)
		m_pathToInterpreter = "/usr/local/bin/php-cgi";
	else if (type == PYTHON)
		m_pathToInterpreter = "/usr/bin/python3"; // check this later

	// Check later: should this be full path, or is relative ok...? CHECK SUBJECT
	std::string currentDir = std::filesystem::current_path();
	m_pathToScript = currentDir + "/home" + m_client.parsedRequest.filePath; // remove "home", it's just a test

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

//	for (int i = 0; i < 2; ++i)
//		std::cout << "EXECVE ARG " << i << " IS:\n" << m_argsForExecve[i] << "\n\n";
}

void	CgiHandler::setExecveEnvArr()
{
	std::map<std::string, std::string>	&headerMap = m_client.parsedRequest.headerMap;

	// Set values to env variables
	m_contenLen = "CONTENT_LENGTH=";
	if (headerMap.find("Content-Length") != headerMap.end())
		m_contenLen += headerMap.at("Content-Length");
	m_contenType = "CONTENT_TYPE=";
	if (headerMap.find("Content-Type") != headerMap.end())
		m_contenType += headerMap.at("Content-Type");
	m_queryStr = "QUERY_STRING=" + m_client.parsedRequest.queryString;
	m_pathInfo = "PATH_INFO=" + m_pathToScript; // check this later (full path or not)
	m_requestMethod = "REQUEST_METHOD=" + m_client.parsedRequest.method;
	m_scriptName = "SCRIPT_FILENAME=" + m_pathToScript;
	m_redirectStatus = "REDIRECT_STATUS="; // check this at school
	m_serverProtocol = "SERVER_PROTOCOL=HTTP/1.1";
	m_gatewayInterface = "GATEWAY_INTERFACE=CGI/1.1";

	// Make strings compatible with execve
	m_envArrExecve[0] = (char *) m_contenLen.c_str(); // check the casting later
	m_envArrExecve[1] = (char *) m_contenType.c_str(); // check the casting later
	m_envArrExecve[2] = (char *) m_queryStr.c_str(); // check the casting later
	m_envArrExecve[3] = (char *) m_serverProtocol.c_str();
	m_envArrExecve[4] = (char *) m_gatewayInterface.c_str();
	m_envArrExecve[5] = (char *) m_pathInfo.c_str();
	m_envArrExecve[6] = (char *) m_requestMethod.c_str();
	m_envArrExecve[7] = (char *) m_scriptName.c_str();
	m_envArrExecve[8] = (char *) m_redirectStatus.c_str();

	for (int i = 0; i < 9; ++i)
		std::cout << "ENV VAR " << i << " IS:\n" << m_envArrExecve[i] << "\n\n";

}

/*
	EXECUTE CGI
*/

// Returns -1 on failure
int	CgiHandler::executeCgi()
{
	if (pipe(m_pipeFromCgi) == -1) // make these O_NONBLOCK
	{
		std::cerr << RED << "\nPipe() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Other error handling?
		return (-1);
	}
	if (pipe(m_pipeToCgi) == -1) // make these O_NONBLOCK
	{
		std::cerr << RED << "\nPipe() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Other error handling?
		return (-1);
	}

	pid_t	cgiPid;
	cgiPid = fork();
	if (cgiPid == -1)
	{
		std::cerr << RED << "\nFork() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		close(m_pipeFromCgi[0]);
		close(m_pipeFromCgi[1]);
		close(m_pipeToCgi[0]);
		close(m_pipeToCgi[1]);
		// Other error handling?
		return (-1);
	}

	if (cgiPid == 0)
		return (cgiChildProcess());
	else
	{
		// In parent/main process

		close(m_pipeFromCgi[1]); // error handling...?
		close(m_pipeToCgi[0]); // error handling...?

		if (m_client.parsedRequest.method == "POST")
		{
			const char *buf = m_client.parsedRequest.rawContent.c_str();
			size_t len = m_client.parsedRequest.rawContent.length();

			std::cout << RED << "BUF IN PARENT:\n" << RESET << buf  << "\nLen:\n" << m_client.parsedRequest.rawContent.length() << "\n";

			write(m_pipeToCgi[1], buf, len); // error handling...?
			close(m_pipeToCgi[1]); // error handling...?
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
		bytesRead = read(m_pipeFromCgi[0], buffer, 1023); // This needs to go thorugh poll()...?
		if (bytesRead == -1)
		{
			std::cerr << RED << "\nRead() failed:\n" << RESET << std::strerror(errno) << "\n\n";
			close(m_pipeFromCgi[0]);
			// Other error handling?
			return (-1);
		}

		buffer[bytesRead] = '\0';

		std::string bufStr = buffer;
		m_responseBody += bufStr;

		/*
			NOTE:

			We need to also check in the ConnectionHandler the Cgi status somehow.
			I mean, when m_responseReady == true, only then we send the response!
		*/
		if (bytesRead < 1023)
		{
			buildCgiResponse();
			m_responseReady = true;
		}

		close(m_pipeFromCgi[0]);
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

	if (chdir(scriptDirectoryPath.c_str()) == -1)
	{
		std::cerr << RED << "\nChdir() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Other error handling?
		return (1);
	}

	close (m_pipeFromCgi[0]); // do we need to check for errors with close()...?
	close (m_pipeToCgi[1]); // error handling...?

	if (dup2(m_pipeFromCgi[1], STDOUT_FILENO) == -1)
	{
		std::cerr << RED << "\nDup2() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Other error handling?
		return (1);	
	}

	
	if (m_client.parsedRequest.method == "POST" && dup2(m_pipeToCgi[0], STDIN_FILENO) == -1)
	{
		std::cerr << RED << "\nDup2() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Other error handling?
		return (1);	
	}


	if (execve(m_pathToInterpreter.c_str(), m_argsForExecve, m_envArrExecve) == -1)
	{
		close(m_pipeFromCgi[1]);
		std::cerr << RED << "\nExecve() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Other error handling?
		return (1); // or some other exit code...?
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
			kill(cgiPid, SIGKILL); // Or sigint...?
			std::cerr << RED << "\nCGI failed due to time out\n" << RESET << "\n";
			// Other error handling?
			return (-1);
		}
		waitpidStatus = waitpid(cgiPid, &statloc, WNOHANG);
	}

	if (waitpidStatus == -1)
	{
		std::cerr << RED << "\nWaitpid() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Other error handling?
		return (-1);
	}

	if (WIFEXITED(statloc) == 1)
	{
		if (WEXITSTATUS(statloc) != 0)
			return (-1); // Or should we return the error code...? Does it matter?
	}
	else if (WIFSIGNALED(statloc) == 1)
	{
		if (WTERMSIG(statloc) == 2)
			std::cout << "Cgi child process got interrupted by SIGINT" << "\n"; // what should we do...?
		else if (WTERMSIG(statloc) == 3)
			std::cout << "Cgi child process got interrupted by SIGQUIT" << "\n"; // what should we do...?
		return (-1);
	}

	return (0);
}

void	CgiHandler::buildCgiResponse()
{
	// Remove extra headers created by php-cgi
	size_t index = m_responseBody.find_first_of('\n');
	index = m_responseBody.find_first_of('\n', index + 1);
	m_responseBody = m_responseBody.substr(index + 1, m_responseBody.length() - index);

	// Make headers & combine response
	m_responseHeaders = "HTTP/1.1 200 OK\r\n";
	m_responseHeaders += "Content-Type: text/html\r\n"; // check this
	m_responseHeaders += "Content-Length: ";
	m_responseHeaders += std::to_string(m_responseBody.length());
	m_responseHeaders += "\r\n\r\n";
	m_client.responseString = m_responseHeaders + m_responseBody;

//	std::cout << "\n\nRESPONSE:\n" << m_client.responseString << "\n\n";
}
