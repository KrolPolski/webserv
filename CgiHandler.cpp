#include "CgiHandler.hpp"

/*
	CONSTRUCTOR
*/

CgiHandler::CgiHandler(clientInfo &client) : m_client(client)
{
	m_scriptReady = false;
	m_pipeReadFinished = false;
	m_cgiTimeOut = 3.0f;

	CgiTypes 	type = m_client.parsedRequest.cgiType;

	if (type == PHP)
		m_pathToInterpreter = "/usr/bin/php";
	else if (type == PYTHON)
		m_pathToInterpreter = "/usr/bin/python3"; // check this later

	// Check later: should this be full path, or is relative ok...? CHECK SUBJECT
	m_pathToScript = "." + m_client.parsedRequest.filePath;

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

	for (int i = 0; i < 2; ++i)
		std::cout << "EXECVE ARG " << i << " IS:\n" << m_argsForExecve[i] << "\n\n";
}

void	CgiHandler::setExecveEnvArr()
{
	// How to check if these headers don't exist??
	m_contenLen = "CONTENT_LENGTH=" + m_client.parsedRequest.headerMap.at("Content-Length");
	m_envArrExecve[0] = (char *) m_contenLen.c_str(); // check the casting later

	m_contenType = "CONTENT_TYPE=" + m_client.parsedRequest.headerMap.at("Content-Type");
	m_envArrExecve[1] = (char *) m_contenType.c_str(); // check the casting later

	m_queryStr = "QUERY_STRING=" + m_client.parsedRequest.queryString;
	m_envArrExecve[2] = (char *) m_queryStr.c_str(); // check the casting later

	m_envArrExecve[3] = NULL;


	for (int i = 0; i < 3; ++i)
		std::cout << "ENV VAR " << i << " IS:\n" << m_envArrExecve[i] << "\n\n";

}

