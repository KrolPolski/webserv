#pragma once

#include "Types.hpp"
#include <filesystem> // for current directory path
#include <cstring> // for errno
#include <unistd.h> // fork()
#include <sys/wait.h> // waitpid()
#include <sys/stat.h> // stat()
#include <poll.h> // poll()
#include <vector>
#include <fcntl.h>

struct serverInfo;
struct clientInfo;

class CgiHandler
{
	public:

	CgiHandler(clientInfo *clientPTR);
	~CgiHandler() {};

	int	writeToCgiPipe(clientInfo *clientPTR);
	int	executeCgi(clientInfo *clientPTR, std::vector<serverInfo> &serverVec, std::vector<clientInfo> &clientVec);
	int	checkWaitStatus(clientInfo *clientPTR);
	int	buildCgiResponse(clientInfo *clientPTR);
	int finishCgiResponse(clientInfo *clientPTR);
	void setPipeToCgiReadReady(void);
	void setPipeFromCgiWriteReady(void);
	pid_t &getCgiChildPid();


	private:

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
	bool		m_scriptReady;
	bool		m_responseReady;

	std::string 	m_pathToInterpreter;
	std::string 	m_pathToScript;

	std::string 	m_responseHeaders;
	std::string 	m_responseBody;
	std::string		m_responseString;

	char 	*m_argsForExecve[3] = {};
	char 	*m_envArrExecve[16] = {};

	pid_t	m_childProcPid;

	void	setExecveArgs();
	void	setExecveEnvArr(clientInfo *clientPTR);

	void	cgiChildProcess(clientInfo *clientPTR, std::vector<serverInfo> serverVec, std::vector<clientInfo> clientVec);

	int		errorExit(clientInfo *clientPTR, std::string errStr, bool isChildProc);
	void	childErrorExit(clientInfo *clientPTR, std::string errStr, bool isChildProc);
	void	closeAndInitFd(int &fd);
	void	closeAndDeleteClient(clientInfo *clientPTR);

	int 	getCgiResponseBodyLen();
	void	closeExtraFD(clientInfo *clientPTR, std::vector<serverInfo> &serverVec, std::vector<clientInfo> &clientVec);

};