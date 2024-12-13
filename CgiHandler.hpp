#pragma once

#include "Types.hpp"
#include "ResponseHandler.hpp"
#include <chrono> // for timer building

class CgiHandler
{
	public:

	CgiHandler(ResponseHandler &obj);
	~CgiHandler() {};

	int	executeCgi(clientInfo *clientPTR, std::string filepath, CgiTypes type);

//////

	private:

	ResponseHandler	&requestData;
	CgiTypes		type;

	bool		m_scriptReady; // meaning the child process has finished
	bool		m_pipeReadFinished; // When read() returns < buffersize

	float		m_cgiTimeOut; // How long we wait for the script to finish

	std::string 	m_scriptPath;
	std::string 	m_headers;
	std::string 	m_body;
	std::string		m_responseString;

	char 	*m_argsForExecve[2];
	char 	*m_envArrExecve[16];

	std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;

	// setArgs()
	// setEnvArr()




};