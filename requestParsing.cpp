#include "ConnectionHandler.hpp"

/*
	ERROR HANDLING AND VALID REQUEST CHECKING...?
*/

// A helper function to split the start line
void	splitStartLine(requestParseInfo	&parseInfo)
{
	size_t		startIndex = 0;
	size_t		endIndex = 0;

	// Extract HTTP method from start line
	endIndex = parseInfo.startLine.find_first_of(' ');
	parseInfo.method = parseInfo.startLine.substr(0, endIndex);
	startIndex = endIndex + 1;

	// Extract requested file path (and possible query string) from start line
	std::string tempStr;

	endIndex = parseInfo.startLine.find_first_of(' ', startIndex);
	tempStr = parseInfo.startLine.substr(startIndex, endIndex - startIndex);
	startIndex = endIndex + 1;

	if (tempStr.find_first_of('?') == tempStr.npos)
		parseInfo.filePath = tempStr; // add also the root folder to the beginning!
	else
	{
		endIndex = tempStr.find_first_of('?');
		parseInfo.filePath = tempStr.substr(0, endIndex); // add also the root folder to the beginning!
		endIndex++;
		parseInfo.queryString = tempStr.substr(endIndex, tempStr.length() - endIndex);
	}

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

	// Do I have to extraxt also the protocol and check that it is indeed HTTP/1.1...?
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

//	std::cout << "REQUEST:\n" << reqStr << "\n";

	// Separate start line from client's request
	endIndex = reqStr.find_first_of('\n');
	if (endIndex == reqStr.npos)
	{
		std::cout << RED << "\nInvalid request: no new line found\n\n" << RESET;
		return (-1);
	}
	parseInfo.startLine = reqStr.substr(0, endIndex - startIndex - 1); // -1 because of '\r'
	startIndex = endIndex + 1;
	splitStartLine(parseInfo);

	// Set header map
	std::string headerLine = "";
	std::string key = "";
	std::string value = "";
	size_t		headerIndex = 0;
	std::map<std::string, std::string> &headerMap = parseInfo.headerMap;

	while (reqStr[startIndex] != '\0')
	{
		endIndex = reqStr.find_first_of('\n', startIndex); // -1 because of '\r'
		if (endIndex == reqStr.npos) // check this later
		{
			std::cout << RED << "\nInvalid request: no new line found after header\n\n" << RESET;
			return (-1);
		}
		headerLine = reqStr.substr(startIndex, endIndex - startIndex - 1);
		headerIndex = headerLine.find_first_of(':');
		if (headerIndex == headerLine.npos)
		{
			std::cout << RED << "\nInvalid request: header is missing ':' character\n\n" << RESET;
			return (-1);
		}
		key = headerLine.substr(0, headerIndex);
		value = headerLine.substr(headerIndex + 2, headerLine.length() - key.length() - 2);
		headerMap[key] = value;
		startIndex = endIndex + 1;
		if (reqStr[startIndex] == '\0' || reqStr[startIndex + 1] == '\n')
			break ;
	}

	// Get content from request

	std::map<std::string, std::string>::iterator it = headerMap.find("Content-Length");
	// OR "Transfer-Encoding: chunked" !!

	if (it != headerMap.end())
	{
		int	contenLen = std::stoi(it->second);
		startIndex += 2; // 2 because we first move to the nl and then over it
		parseInfo.rawContent = reqStr.substr(startIndex, contenLen);

//		std::cout << RED << "RAW CONTENT:\n" << RESET << parseInfo.rawContent << "\n";

	}

	return (0);
}

