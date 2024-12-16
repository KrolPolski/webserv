#pragma once

#include "Types.hpp"
#include "ResponseHandler.hpp"
#include <chrono> // for timer building

class CgiHandler
{
	public:

	CgiHandler(clientInfo &client);
	~CgiHandler() {};

	int	executeCgi(clientInfo *clientPTR, std::string filepath, CgiTypes type);

//////

	private:

	clientInfo	&m_client;

	std::string m_contenLen;
	std::string m_contenType;
	std::string m_queryStr;


	bool		m_scriptReady; // meaning the child process has finished
	bool		m_pipeReadFinished; // When read() returns < buffersize
	float		m_cgiTimeOut; // How long we wait for the script to finish

	std::string m_pathToInterpreter;
	std::string m_pathToScript;

	std::string 	m_responseHeaders;
	std::string 	m_responseBody;
	std::string		m_responseString;

	char 	*m_argsForExecve[3] = {}; // is this initialization ok?
	char 	*m_envArrExecve[16] = {};

	std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;

	void setExecveArgs();
	void setExecveEnvArr();

};