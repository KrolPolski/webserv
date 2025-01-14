#include "ResponseHandler.hpp"
#include "CgiHandler.hpp"
#include "Structs.hpp"
#include <iostream>

const std::map<std::string, std::string> ResponseHandler::extensionTypes = 
{
	{".aac", "audio/aac"}, {".abw", "application/x-abiword"},
	{".apng", "image/apng"}, {".arc", "application/x-freearc"},
	{".avif", "image/avif"}, {".avi", "video/x-msvideo"},
	{".azw", "application/vnd.amazon.ebook"}, {".bin", "application/octet-stream"},
	{".bmp", "image/bmp"}, {".bz", "application/x-bzip"},
	{".bz2", "application/x-bzip2"}, {".cda", "application/x-cdf"},
	{".csh", "application/x-csh"}, {".css",	"text/css"},
	{".csv", "text/csv"}, {".doc", "application/msword"},
	{".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
	{".eot", "application/vnd.ms-fontobject"}, {".epub", "application/epub+zip"},
	{".gz", "application/gzip"}, {".gif", "image/gif"},
	{".htm", "text/html"}, {".html", "text/html"},
	{".ico", "image/vnd.microsoft.icon"}, {".ics", "text/calendar"},
	{".jar", "application/java-archive"}, {".jpeg", "image/jpeg"},
	{".jpg", "image/jpeg"}, {".js","text/javascript"},
	{".json", "application/json"}, {".jsonld", "application/ld+json"},
	{".md", "text/markdown"},
	{".mid", "audio/x-midi"}, {".midi",	"audio/x-midi"},
	{".mjs", "text/javascript"}, {".mp3", "audio/mpeg"},
	{".mp4", "video/mp4"}, {".mpeg", "video/mpeg"},
	{".mpkg", "application/vnd.apple.installer+xml"}, {".odp", "application/vnd.oasis.opendocument.presentation"},
	{".ods", "application/vnd.oasis.opendocument.spreadsheet"},
	{".odt", "application/vnd.oasis.opendocument.text"},
	{".oga", "audio/ogg"}, {".ogv", "video/ogg"},
	{".ogx", "application/ogg"}, {".opus", "audio/ogg"},
	{".otf", "font/otf"}, {".png", "image/png"},
	{".pdf", "application/pdf"}, {".php", "application/x-httpd-php"},
	{".ppt", "application/vnd.ms-powerpoint"}, {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
	{".rar", "application/vnd.rar"}, {".rtf", "application/rtf"},
	{".sh", "application/x-sh"}, {".svg", "image/svg+xml"}, 
	{".tar", "application/x-tar"}, {".tif", "image/tiff"},
	{".tiff", "image/tiff"}, {".ts", "video/mp2t"},
	{".ttf", "font/ttf"}, {".txt", "text/plain"},
	{".vsd", "application/vnd.visio"}, {".wav", "audio/wav"},
	{".weba", "audio/webm"}, {".webm", "video/webm"},
	{".webp", "image/webp"}, {".woff", "font/woff"},
	{".woff2", "font/woff2"}, {".xhtml", "application/xhtml+xml"},
	{".xls", "application/vnd.ms-excel"}, {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
	{".xml", "application/xml"}, {".xul", "application/vnd.mozilla.xul+xml"},
	{".zip", "application/zip"}, {".3gp", "video/3gpp"},
	{".3g2", "video/3gpp2"}, {".7z", "application/x-7z-compressed"}
};

const std::map<const unsigned int, std::string> ResponseHandler::errorCodes =
{
	{404, "Not Found"},
	{403, "Forbidden"},
	{405, "Method Not Allowed"},
	{400, "Bad Request"},
	{500, "Internal Server Error"}
};

void ResponseHandler::checkRequestType(clientInfo *ClientPTR, std::string requestString)
{    
//  std::cout << "We managed to get to checkRequestType" << std::endl;
//  std::cout << requestString << std::endl;
//	std::cout << "We should have printed the http request by now" << std::endl;

	if (!requestString.compare(0, 4, "GET "))
	{
		//std::cout << "GET request detected woo" << std::endl;
		setRequestType(GET);
	}
	else if (!requestString.compare(0, 5, "POST "))
	{
		//std::cout << "POST request detected woo" << std::endl;
		setRequestType(POST);
	}
	else if (!requestString.compare(0, 7, "DELETE "))
	{
		//std::cout << "DELETE request detected woo" << std::endl;
		setRequestType(DELETE);
	}
	else
	{
		//std::cout << "INVALID request detected woo" << std::endl;
		setRequestType(INVALID);
		setResponseCode(400);
		openErrorResponseFile(ClientPTR);
	}
}

void ResponseHandler::checkExtension(std::string filePath)
{
	//std::cout << "Inside checkExtension" << std::endl;
	std::string extension;
	size_t index;
	index = filePath.find_last_of('.');
	if (index == std::string::npos)
	{
		contentType = "Unknown";
		return ;
	}
	extension = filePath.substr(index, filePath.length());
	if (extension.length() < 2 || extensionTypes.find(extension) == extensionTypes.end())
	{
		contentType = "Unknown";
		return ;
	}

	//std::cout << "filePath: " << filePath << " Extension: " << extension << " Type: " << extensionTypes.at(extension) << std::endl;
	try
	{
		contentType = extensionTypes.at(extension);
	//	std::cout << "contentType: " << contentType << std::endl;
	} 
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		//there's probably a response code for this
	}
	//std::cout << "exiting checkExtension" << std::endl;
}

void ResponseHandler::buildRedirectResponse(std::string webFilePath, clientInfo *clientPTR)
{
// Example response:
//	HTTP/1.1 301 Moved Permanently
// Location: http://example.com/folder/
// Content-Type: text/html; charset=UTF-8
// Content-Length: 0
// Connection: close
	setResponseCode(301);
	std::string headers;
	headers = "HTTP/1.1 301 Moved Permanently\r\n";
	headers += "Location: "	+ webFilePath + "\r\n";
	headers += "Content-Type: text/html;" "\r\n";
	headers += "Content-Length: 0\r\n";
	headers += "Connection: close";
	headers += "\r\n\r\n";
	clientPTR->responseString = headers;

	clientPTR->status = SEND_RESPONSE;
//	std::cout << "responseString: " << clientPTR->responseString << std::endl;
}

int ResponseHandler::openResponseFile(clientInfo *clientPTR, std::string filePath)
{
	checkExtension(filePath); // is this a good place for this...?

	clientPTR->responseFileFd = open(filePath.c_str(), O_RDONLY);
	if (clientPTR->responseFileFd == -1)
	{
		std::cerr << RED << "\nopen() of response file failed:\n" << RESET << std::strerror(errno) << "\n\n";
		if (errno == 2) //file missing
			setResponseCode(404);
		else if (errno == 13) //bad permissions
			setResponseCode(403);
		else
			setResponseCode(500);
		std::cerr << "Response Code: " << getResponseCode() << std::endl;
		openErrorResponseFile(clientPTR);
	}
	else if (fcntl(clientPTR->responseFileFd, F_SETFL, O_NONBLOCK) == -1) // make file fd non-blocking
	{
		std::cerr << RED << "\nfcntl() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		setResponseCode(500); // is this ok...?
		openErrorResponseFile(clientPTR);
	}
	else
		clientPTR->status = BUILD_REPONSE;

	return (0);
}

int ResponseHandler::openCgiPipes(clientInfo *clientPTR)
{
	if (pipe(clientPTR->pipeToCgi) == -1 || pipe(clientPTR->pipeFromCgi) == -1)
	{
		std::cerr << RED << "\npipe() in CGI failed:\n" << RESET << std::strerror(errno) << "\n\n";
		setResponseCode(500); // is this ok...?
		openErrorResponseFile(clientPTR);
		return (-1); // Might not be needed
	}
	else if (fcntl(clientPTR->pipeToCgi[0], F_SETFL, O_NONBLOCK) == -1
	|| fcntl(clientPTR->pipeToCgi[1], F_SETFL, O_NONBLOCK) == -1
	|| fcntl(clientPTR->pipeFromCgi[0], F_SETFL, O_NONBLOCK) == -1
	|| fcntl(clientPTR->pipeFromCgi[1], F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << RED << "\nfcntl() in CGI failed:\n" << RESET << std::strerror(errno) << "\n\n";
		setResponseCode(500); // is this ok...?
		openErrorResponseFile(clientPTR);
		return (-1); // Might not be needed
	}
	else
		clientPTR->status = EXECUTE_CGI;

	return (0);
}

int ResponseHandler::checkFile(clientInfo *clientPTR, std::string filePath)
{
  
//    std::cout << "We should now check if the file exists" << std::endl;
	// we need to have separate variables for the local path vs the web path so redirects work the way they should.
	std::string defaultFilePath;
	std::string webFilePath;
	bool dirListing {false};

	filePath = clientPTR->parsedRequest.filePath;
	webFilePath = filePath;
	std::string port = clientPTR->relatedServer->serverConfig->getPort();
	//std::cout << "Port returned: " << port << std::endl;
	filePath = clientPTR->relatedServer->serverConfig->getRoot("/") + filePath;
	//std::cout << "Updated file path is: " << filePath << std::endl;
	
	std::string content;
	if (std::filesystem::is_directory(filePath))
	{
		if (filePath.back() == '/')
			defaultFilePath = filePath + "index.html";
		else
		{
			webFilePath += "/";
			buildRedirectResponse(webFilePath, clientPTR);
			return 0;
		}
		if (std::filesystem::exists(defaultFilePath))
			filePath = defaultFilePath;
		else
		{
			//std::cout <<"Call Patrik's function for directory listing" << std::endl; // DirListing needs to be checked for if ON/OFF ---- Patrik
			// Please call for dir perms using the web path, not the actual path
			dirListing = clientPTR->relatedServer->serverConfig->getDirListing(webFilePath);
			//std::cout << "For path: " << webFilePath << " = " << dirListing << std::endl;
			if (dirListing)
				buildDirListingResponse(filePath, clientPTR);
			else
			{
				setResponseCode(403);
				openErrorResponseFile(clientPTR);
			}
			return 0;
		}
	}
	// If we get here then we have concluded that it isn't a directory.

	
	if (clientPTR->parsedRequest.isCgi)
	{
		m_cgiHandler = new CgiHandler(*clientPTR);

		if (openCgiPipes(clientPTR) == -1)
			return (-1); // this might be unnecessary, since the return value of checkFile() is not checked!<
		
		return (0);
	}

	
	return (openResponseFile(clientPTR, filePath));

}

bool ResponseHandler::checkRequestAllowed(clientInfo *clientPTR, std::string filePath)
{
	(void)filePath;
	std::string permittedMethods = clientPTR->relatedServer->serverConfig->getMethods(filePath);
	//std::cout << "We decided permitted Methods are: " << permittedMethods << std::endl;
	//std::cout << "Our request type is " << requestTypeAsString << std::endl;
	if (permittedMethods.find(requestTypeAsString) != std::string::npos)
	{
		//std::cout << "Method must have been permitted" << std::endl;
		return true;
	}
	else
	{
		std::cout << "Method not permitted, should show 405 page" << std::endl;
		responseCode = 405; // This is being set the second time in the calling function
		return false;
	}
}

// This could be named something like "isResponseValid" etc, referring to the extra checks
// And I'd just use the parsing I did in the other function, that is stored in clientPTR->parsedRequest
void ResponseHandler::parseRequest(clientInfo *clientPTR, std::string requestString)
{
	switch (requestType)
	{
		case GET:
		{
			requestTypeAsString = "GET";
			std::istringstream stream(requestString);
			std::vector<std::string> reqVec;
			std::string line;
//			std::cout <<"\nBeginning line splits" << std::endl;
			while (std::getline(stream, line, '\n'))
			{
				reqVec.push_back(line);
//				std::cout << line << std::endl;
			}
//			std::cout << "Line splits done" << std::endl;
//			std::cout << "Line one is: " << reqVec.at(0) << std::endl;
			std::istringstream streamL1(reqVec.at(0));
			std::string phrase; 
			std::vector<std::string> lineOne;
//			std::cout << "\nBeginning phrase split" << std::endl;
			while (std::getline(streamL1, phrase, ' '))
			{
				lineOne.push_back(phrase);
//				std::cout << phrase << std::endl;
			}
			if (lineOne.size() >= 2)
				{
					if (checkRequestAllowed(clientPTR, lineOne.at(1)))
						checkFile(clientPTR, lineOne.at(1)); // we need to check return value here in case something goes wrong
					else
					{
						setResponseCode(405);
						openErrorResponseFile(clientPTR);
					}
				}
			break;
		}	
		case POST:
		{
			checkFile(clientPTR, ""); // JUST A TEST
			break ;
		}
		default:
			std::cout << "unhandled parseRequest" << std::endl;
	}
}

int ResponseHandler::buildResponse(clientInfo *clientPTR)
{
	if (clientPTR->responseFileFd == -1)
		std::cout << RED << "\nERROR! buildResponse called before response file was opened\n" << RESET;

	char 	buffer[1024];
	int		bytesRead;
	int		readPerCall = 1023;

	bytesRead = read(clientPTR->responseFileFd, buffer, readPerCall);
	if (bytesRead == -1)
	{
		std::cerr << RED << "\nread() of error page file failed:\n" << RESET << std::strerror(errno) << "\n\n";
		clientPTR->status = DISCONNECT; // --> Not good, we need a standard error page here!
		return (-1);
	}

	buffer[bytesRead] = '\0';
	clientPTR->responseBody += buffer;

	if (bytesRead < readPerCall)
	{
		std::string	headers;
		setResponseCode(200);
		headers = "HTTP/1.1 200 OK\r\n";
		headers += "Content-Type: " + contentType + "\r\n";
		headers += "Content-Length: ";
		headers += std::to_string(clientPTR->responseBody.length());
		headers += "\r\n\r\n";
		clientPTR->responseString = headers + clientPTR->responseBody;
		clientPTR->status = SEND_RESPONSE;

		std::cout << RED << "Built response:\n\n" << RESET << clientPTR->responseString << "\n\n";

	}

//	std::cout << "responseString: " << clientPTR->responseString << std::endl;

	return (0);
}

const enum requestTypes& ResponseHandler::getRequestType() const
{
	return requestType;
}

unsigned int ResponseHandler::getResponseCode() const
{
	return responseCode;
}

void ResponseHandler::setRequestType(enum requestTypes reqType)
{
	requestType = reqType;

}

void ResponseHandler::setResponseCode(unsigned int code)
{
	responseCode = code;
}

void ResponseHandler::ServeErrorPages(clientInfo *ClientPTR, std::string requestString)
{
	// This is just to satisfy compiler
	if (ClientPTR == NULL || requestString == "")
		return ;
}

void ResponseHandler::openErrorResponseFile(clientInfo *clientPTR)
{
	std::string errorFileName = "home/error/" + std::to_string(getResponseCode()) + ".html"; // HARD CODED!!
	checkExtension(errorFileName); // Is this a good place for this...?

	clientPTR->errorFileFd = open(errorFileName.c_str(), O_RDONLY);
	if (clientPTR->errorFileFd == -1)
	{
		// Do we need to specify what went wrong with the opening...?
		std::cerr << RED << "\nopen() of error page file failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Should we have a backup error page here...? That is built within our code?
		// If for example someone were to mess with all the permissions of our error pages...

		// Disconnect is not good. Here we need to build some kind of "general error page" within our code !!
		clientPTR->status = DISCONNECT;
	}
	else if (fcntl(clientPTR->errorFileFd, F_SETFL, O_NONBLOCK) == -1) // make file fd non-blocking
	{
		std::cerr << RED << "\nfcntl() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Disconnect is not good. Here we need to build some kind of "general error page" within our code !!
		clientPTR->status = DISCONNECT;
	}
	else
		clientPTR->status = BUILD_ERRORPAGE;
}


void ResponseHandler::buildErrorResponse(clientInfo *clientPTR)
{
	if (clientPTR->errorFileFd == -1)
		std::cout << RED << "\nERROR! buildErrorResponse called before error file was opened\n" << RESET;

//	std::cout << "We got to buildErrorResponse" << std::endl;
	//need to update this based on config file path to error pages
	//tested with this and it works. now just fetch this from the configuration data. --- Patrik


	char 	buffer[1024];
	int		bytesRead;
	int		readPerCall = 1023;

	bytesRead = read(clientPTR->errorFileFd, buffer, readPerCall);
	if (bytesRead == -1)
	{
		std::cerr << RED << "\nread() of error page file failed:\n" << RESET << std::strerror(errno) << "\n\n";
		clientPTR->status = DISCONNECT; // --> Not good, we need a standard error page here!
		return ;
	}

	buffer[bytesRead] = '\0';
	clientPTR->responseBody += buffer;

	if (bytesRead < readPerCall)
	{
		std::string	headers;

		headers = "HTTP/1.1 " + std::to_string(getResponseCode()) + " " + errorCodes.at(getResponseCode()) + "\r\n";
		headers += "Content-Type: " + contentType + "\r\n";
		headers += "Content-Length: ";
		headers += std::to_string(clientPTR->responseBody.length());
		headers += "\r\n\r\n";
		clientPTR->responseString = headers + clientPTR->responseBody;
		std::cout << "responseString: " << clientPTR->responseString << std::endl;

		clientPTR->status = SEND_RESPONSE;
	}

}

void	ResponseHandler::buildDirListingResponse(const std::string& pathForDirToList, clientInfo *clientPTR)
{
	std::string	content;
	content += "<html>\n<body>\n";
	content += " <h1>In directory with path: " + pathForDirToList + "</h1>\n";
	content += "<ul>\n";

	// is filesystem considered a "reading operation"...? Do we need to go through poll() before that?
	for(const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(pathForDirToList))
	{
		std::string href = entry.path().filename().string();
		std::string name = entry.path().filename().string();

		content += "  <li><a href=\"" + href + "\">" + name + "</a></li>\n";
	}
	content += "</ul>\n</body>\n</html>\n";

	std::string headers;
	contentType = "text/html";

	headers = "HTTP/1.1 200 OK\r\n";
	headers += "Content-Type: " + contentType + "\r\n";
	headers += "Content-Length: ";
	headers += std::to_string(content.length());
	headers += "\r\n\r\n";
	clientPTR->responseString = headers + content;
	clientPTR->status = SEND_RESPONSE;
}
