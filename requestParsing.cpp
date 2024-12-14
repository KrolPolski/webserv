#include "ConnectionHandler.hpp"

/*
	ERROR HANDLING AND VALID REQUEST CHECKING...?
*/

// A helper function to split the start line
void	splitStartLine(requestParseInfo	&parseInfo)
{
	size_t		startIndex = 0;
	size_t		endIndex = 0;

	// Extract method from start line
	endIndex = parseInfo.startLine.find_first_of(' ');
	parseInfo.method = parseInfo.startLine.substr(0, endIndex);
	startIndex = endIndex + 1;

	// Extract file path (and possible query string) from start line
	std::string tempStr;

	endIndex = parseInfo.startLine.find_first_of(' ', startIndex);
	tempStr = parseInfo.startLine.substr(startIndex, endIndex - startIndex);
	startIndex = endIndex + 1;

	if (tempStr.find_first_of('?') == tempStr.npos)
		parseInfo.requestedFilePath = tempStr;
	else
	{
		endIndex = tempStr.find_first_of('?');
		parseInfo.requestedFilePath = tempStr.substr(0, endIndex);
		endIndex++;
		parseInfo.queryString = tempStr.substr(endIndex, tempStr.length() - endIndex);
	}

	// Do I have to extraxt also the protocol and check that it is indeed HTTP/1.1...?

}

/*
	Parses HTTP request as follows:

	1. Splits start line and extracts method and filepath (+ possible query string)
	2. Parses all headers into a map (key = header name, value = header value)
	3. Extracts content from the request body (if one is found)
*/
int		ConnectionHandler::parseBlocks(clientInfo *clientPTR)
{
	size_t			startIndex = 0;
	size_t			endIndex = 0;
	std::string 	&reqStr = clientPTR->requestString;
	requestParseInfo	&parseInfo = clientPTR->parsedRequest;

//	std::cout << "REQUEST:\n" << reqStr << "\n\n";

	// Separate start line from request

	endIndex = reqStr.find_first_of('\n');
	if (endIndex == reqStr.npos)
	{
		std::cout << RED << "\nInvalid request: no new line found\n\n" << RESET;
		return (-1);
	}
	parseInfo.startLine = reqStr.substr(0, endIndex - startIndex);
	startIndex = endIndex + 1;

//	std::cout << "START LINE:\n" << parseInfo.startLine << "\n\n";

	splitStartLine(parseInfo);

	std::cout << GREEN << "Method:\n" << parseInfo.method 
	<< "\nFilepath:\n" << parseInfo.requestedFilePath << "\nQuery string:\n" << parseInfo.queryString
	<< RESET << "\nEnd of test\n"; 

	// Set header map

	std::string headerLine = "";
	std::string key = "";
	std::string value = "";
	size_t		headerIndex = 0;
	std::map<std::string, std::string> &headerMap = parseInfo.headerMap;

	while (reqStr[startIndex] != '\0')
	{
		endIndex = reqStr.find_first_of('\n', startIndex);
		if (endIndex == reqStr.npos) // check this later
		{
			std::cout << RED << "\nInvalid request: no new line found after header\n\n" << RESET;
			return (-1);
		}
		headerLine = reqStr.substr(startIndex, endIndex - startIndex);
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
	}

//	std::cout << RED << "CONTENT:\n" << parseInfo.rawContent << "END OF CONTENT\n\n" << RESET;

	return (0);

}

