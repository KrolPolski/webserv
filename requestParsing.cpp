#include "ConnectionHandler.hpp"
#include "URLhandler.hpp"
#include "Logger.hpp"


// A helper function to split the start line
int	ConnectionHandler::splitStartLine(clientInfo *clientPTR, requestParseInfo	&parseInfo)
{
	URLhandler	urlHandler;
	size_t		startIndex = 0;
	size_t		endIndex = 0;

	// Extract HTTP method from start line
	endIndex = parseInfo.startLine.find_first_of(' ');
	if (endIndex == parseInfo.startLine.npos)
	{
		webservLog.webservLog(ERROR, "Invalid request: first line is missing URI", true);
		clientPTR->respHandler->setResponseCode(400);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		return (-1);
	}

	parseInfo.method = parseInfo.startLine.substr(0, endIndex);
	startIndex = endIndex + 1;

	// Extract requested file path (and possible query string) from start line
	std::string tempStr;

	endIndex = parseInfo.startLine.find_first_of(' ', startIndex);
	if (endIndex == parseInfo.startLine.npos)
	{
		webservLog.webservLog(ERROR, "Invalid request: first line is missing HTTP protocol", true);
		clientPTR->respHandler->setResponseCode(400);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		return (-1);
	}
	tempStr = parseInfo.startLine.substr(startIndex, endIndex - startIndex);
	startIndex = endIndex + 1;

	if (tempStr.size() > 4096) // From ChatGPT: "NGINX: The default maximum URI length is 4,096 characters (4 KB)"
	{
		webservLog.webservLog(ERROR, "Invalid request: URI too long", true);
		clientPTR->respHandler->setResponseCode(414);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		return (-1);
	}

	if (tempStr.find_first_of('?') == tempStr.npos)
		parseInfo.filePath = tempStr;
	else
	{
		endIndex = tempStr.find_first_of('?');
		parseInfo.filePath = tempStr.substr(0, endIndex); // add also the root folder to the beginning!
		endIndex++;
		parseInfo.queryString = tempStr.substr(endIndex, tempStr.length() - endIndex);
	}

	urlHandler.decode(parseInfo.filePath);

	// Parse extension
	startIndex = parseInfo.filePath.find_last_of('.');
	if (startIndex != parseInfo.filePath.npos)
		parseInfo.extension = parseInfo.filePath.substr(startIndex, parseInfo.filePath.length() - startIndex);

	// Check CGI
	if (parseInfo.extension == ".php")
	{
		parseInfo.isCgi = true;
		parseInfo.cgiType = PHP;
	}
	else if (parseInfo.extension == ".py")
	{
		parseInfo.isCgi = true;
		parseInfo.cgiType = PYTHON;
	}

	startIndex = parseInfo.startLine.find_last_of(' ') + 1;
	if (parseInfo.startLine.compare(startIndex, 8, "HTTP/1.1") != 0)
	{
		webservLog.webservLog(ERROR, "Invalid request: Bad HTTP protocol. Only HTTP/1.1 is supported", true);
		clientPTR->respHandler->setResponseCode(505);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		return (-1);
	}

	return 0;
}

int ConnectionHandler::getRelatedServer(clientInfo *clientPTR)
{
	size_t reqIdxStart = clientPTR->requestString.find("Host: ");
	if (reqIdxStart == std::string::npos)
	{
		std::cerr << RED << "Invalid request: " << RESET << "Host header is missing" << RESET;
		clientPTR->respHandler->setResponseCode(400);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		return (-1);
	}

	reqIdxStart += 6; // +6 to skip 'Host: '
	size_t reqIdxEnd = clientPTR->requestString.find("\r\n", reqIdxStart);

	std::string requestHost = clientPTR->requestString.substr(reqIdxStart, reqIdxEnd - reqIdxStart);
	size_t columnIdx = requestHost.find(':');
	if (columnIdx != std::string::npos)
		requestHost = requestHost.substr(0, columnIdx);

	std::cout << "ReqHost:\n" << requestHost << "\n";

	for (auto &server : m_serverVec)
	{
		std::cout << "ServerHost:\n" << server.serverConfig->getHost() << "\n";

		if (requestHost == server.serverConfig->getHost())
		{
			std::cout << "We have a match!\n";
			clientPTR->relatedServer = &server;
			return 0;
		}

		std::string serverNames = server.serverConfig->getNames();
		std::string curName;
		size_t endIdx = serverNames.find(' ');
		size_t startIdx = 0;

		while (startIdx < serverNames.size())
		{
			if (endIdx == std::string::npos)
				endIdx = serverNames.size();

			curName = serverNames.substr(startIdx, endIdx - startIdx);

			std::cout << "Server name:\n" << curName << "\n";

			if (requestHost == curName)
			{
				std::cout << "We have a name match!\n";

				clientPTR->relatedServer = &server;
				return 0;
			}

			startIdx = endIdx + 1;
			endIdx = serverNames.find(' ', startIdx);
		}

	}

	clientPTR->relatedServer = &m_serverVec[0];

	return 0;

}

/*
	Parses HTTP request as follows:

	1. Splits start line and extracts method and filepath (+ possible query string)
	2. Parses all headers into a map (key = header name, value = header value)
	3. Extracts content from the request body (if one is found)

	This function could be broken into smaller pieces
*/
int		ConnectionHandler::parseRequest(clientInfo *clientPTR)
{
	size_t			startIndex = 0;
	size_t			endIndex = 0;
	std::string 	&reqStr = clientPTR->requestString;
	requestParseInfo	&parseInfo = clientPTR->parsedRequest;

	std::cout << "REQUEST:\n" << reqStr << "\n\n";

	// Separate start line from client's request
	endIndex = reqStr.find_first_of("\r\n");
	if (endIndex == reqStr.npos)
	{
		webservLog.webservLog(ERROR, "Invalid request: no new line found", true);
		clientPTR->respHandler->setResponseCode(400);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		return (-1);
	}
	parseInfo.startLine = reqStr.substr(0, endIndex - startIndex);
	startIndex = endIndex + 2;
	if (splitStartLine(clientPTR, parseInfo) == -1)
		return (-1);

	// Set header map
	std::string headerLine = "";
	std::string key = "";
	std::string value = "";
	size_t		headerIndex = 0;
	std::map<std::string, std::string> &headerMap = parseInfo.headerMap;

	while (reqStr[startIndex] != '\0')
	{
		endIndex = reqStr.find_first_of("\r\n", startIndex);
		if (endIndex == reqStr.npos) // check this later
		{
			webservLog.webservLog(ERROR, "Invalid request: no new line found after header", true);
			clientPTR->respHandler->setResponseCode(400);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			return (-1);
		}
		headerLine = reqStr.substr(startIndex, endIndex - startIndex);
		headerIndex = headerLine.find_first_of(':');
		if (headerIndex == headerLine.npos)
		{
			webservLog.webservLog(ERROR, "Invalid request: header is missing ':' character", true);
			clientPTR->respHandler->setResponseCode(400);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			return (-1);
		}
		key = headerLine.substr(0, headerIndex);
		value = headerLine.substr(headerIndex + 2, headerLine.length() - key.length() - 2);
		headerMap[key] = value;
		startIndex = endIndex + 2;
		if (reqStr[startIndex] == '\0' || reqStr[startIndex + 1] == '\n')
			break ;
	}

	// Get content from request

	auto it = headerMap.find("Content-Length");

	if (it != headerMap.end())
	{
		int	contenLen = std::stoi(it->second);
		if (contenLen < 0)
		{
			webservLog.webservLog(ERROR, "Invalid request: Content-Length is negative", true);
			clientPTR->respHandler->setResponseCode(400);
			clientPTR->respHandler->openErrorResponseFile(clientPTR);
			return (-1);
		}
		startIndex += 2; // 2 because we first move to the '\n' and then over it
		parseInfo.rawContent = reqStr.substr(startIndex, contenLen);
	}
	else if (it == headerMap.end() && parseInfo.method == "POST")
	{
		webservLog.webservLog(ERROR, "Invalid request: POST request without Content-Length header", true);
		clientPTR->respHandler->setResponseCode(400);
		clientPTR->respHandler->openErrorResponseFile(clientPTR);
		return (-1);
	}

	return (0);
}

