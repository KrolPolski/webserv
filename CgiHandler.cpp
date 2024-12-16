#include "CgiHandler.hpp"

/*
	CONSTRUCTOR
*/

CgiHandler::CgiHandler(clientInfo &client) : m_client(client)
{
	m_scriptReady = false;
	m_pipeReadFinished = false;
	m_cgiTimeOut = 3;

	CgiTypes 	type = m_client.parsedRequest.cgiType;

	if (type == PHP)
		m_pathToInterpreter = "/usr/local/bin/php-cgi";
	else if (type == PYTHON)
		m_pathToInterpreter = "/usr/bin/python3"; // check this later

	// Check later: should this be full path, or is relative ok...? CHECK SUBJECT
	std::string currentDir = std::filesystem::current_path();
	m_pathToScript = currentDir + "/home" + m_client.parsedRequest.filePath; // remove "home", it's just a test

	m_contenLen = "CONTENT_LENGTH="; // this or NULL if header is not found...?
	m_contenType = "CONTENT_TYPE="; // this or NULL if header is not found...?
	m_queryStr = "QUERY_STRING=" + m_client.parsedRequest.queryString;
	m_serverProtocol = "SERVER_PROTOCOL=HTTP/1.1";
	m_gatewayInterface = "GATEWAY_INTERFACE=CGI/1.1";
	m_pathInfo = "PATH_INFO=" + m_pathToScript; // check this later (full path or not)
	m_requestMethod = "REQUEST_METHOD=" + m_client.parsedRequest.method;
	m_scriptName = "SCRIPT_FILENAME=" + m_pathToScript;
	m_redirectStatus = "REDIRECT_STATUS=";

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
	std::map<std::string, std::string>	&headerMap = m_client.parsedRequest.headerMap;

	if (headerMap.find("Content-Length") != headerMap.end())
		m_contenLen += headerMap.at("Content-Length");
	m_envArrExecve[0] = (char *) m_contenLen.c_str(); // check the casting later

	if (headerMap.find("Content-Type") != headerMap.end())
		m_contenType += headerMap.at("Content-Type");
	m_envArrExecve[1] = (char *) m_contenType.c_str(); // check the casting later

	m_envArrExecve[2] = (char *) m_queryStr.c_str(); // check the casting later
	m_envArrExecve[3] = (char *) m_serverProtocol.c_str();
	m_envArrExecve[4] = (char *) m_gatewayInterface.c_str();
	m_envArrExecve[5] = (char *) m_pathInfo.c_str();
	m_envArrExecve[6] = (char *) m_requestMethod.c_str();
	m_envArrExecve[7] = (char *) m_scriptName.c_str();
	m_envArrExecve[8] = (char *) m_redirectStatus.c_str(); // check this at school

	for (int i = 0; i < 9; ++i)
		std::cout << "ENV VAR " << i << " IS:\n" << m_envArrExecve[i] << "\n\n";

}

