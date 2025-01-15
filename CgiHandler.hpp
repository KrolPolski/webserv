#pragma once

#include "Types.hpp"
#include <chrono> // for timer building
#include <filesystem> // for current directory path
#include <cstring> // for errno
#include <unistd.h> // fork()
#include <sys/wait.h> // waitpid()

// TEST
#include <fcntl.h>

struct clientInfo;

class CgiHandler
{
	public:

	CgiHandler(clientInfo &client);
	~CgiHandler() {};

	int	writeToCgiPipe();
	int	executeCgi();
	int	checkWaitStatus();
	int	buildCgiResponse(clientInfo *clientPTR);

	void setPipeToCgiReadReady(void);
	void setPipeFromCgiWriteReady(void);

	pid_t &getCgiChildPid();


//////

	private:

	clientInfo	&m_client;

	std::string m_contenLen;
	std::string m_contenType;
	std::string m_queryStr;
	std::string m_gatewayInterface;
	std::string m_pathInfo;
	std::string m_requestMethod;
	std::string m_serverProtocol;
	std::string	m_scriptFileName;
	std::string	m_scriptName;
	std::string m_redirectStatus;
	std::string m_remote_addr;
	std::string	m_serverName;
	std::string	m_serverPort;

	bool		m_pipeToCgiWriteDone;
	bool		m_pipeToCgiReadReady;
	bool		m_pipeFromCgiWriteReady;
	bool		m_childProcRunning;
	bool		m_scriptReady; // meaning the child process has finished
	bool		m_responseReady; // When read() returns < buffersize --> might not be needed

	std::string 	m_pathToInterpreter;
	std::string 	m_pathToScript;

	std::string 	m_responseHeaders;
	std::string 	m_responseBody;
	std::string		m_responseString;

	char 	*m_argsForExecve[3] = {}; // is this initialization ok?
	char 	*m_envArrExecve[16] = {};

	pid_t	m_childProcPid;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;

	void	setExecveArgs();
	void	setExecveEnvArr();

	int		cgiChildProcess();

	int		errorExit(std::string errStr, bool isChildProc);
	void	closeAndInitFd(int &fd);
	void	closeAndDeleteClient();

};