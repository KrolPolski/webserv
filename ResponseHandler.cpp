#include "ResponseHandler.hpp"
#include "ConnectionHandler.hpp"
#include "Structs.hpp"
#include "Logger.hpp"

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
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{408, "Client Timeout"}, // Is this ok...? -Panu
	{500, "Internal Server Error"}
};

/* Sets the appropriate request type enum */
void ResponseHandler::setRequestType(clientInfo *clientPTR)
{    

	if (clientPTR->parsedRequest.method == "GET")
		setRequestType(GET);
	else if (clientPTR->parsedRequest.method == "POST")
		setRequestType(POST);
	else if (clientPTR->parsedRequest.method == "DELETE")
		setRequestType(DELETE);
	else
	{
		setRequestType(INVALID);
		setResponseCode(400);
		openErrorResponseFile(clientPTR);
	}
}

void ResponseHandler::setExtension(clientInfo *clientPTR)
{
	std::string extension = clientPTR->parsedRequest.extension;
	if (extension == "" || extension.length() < 2 || extensionTypes.find(extension) == extensionTypes.end())
	{
		contentType = "Unknown";
		return ;
	}
	try
	{
		contentType = extensionTypes.at(extension);
	} 
	catch (std::exception& e)
	{
		webservLog.webservLog(ERROR, e.what(), false);
	}
}

/* Builds a 301 response header when required for redirects*/
void ResponseHandler::buildRedirectResponse(std::string webFilePath, clientInfo *clientPTR)
{
	setResponseCode(301);
	std::string headers;
	headers = "HTTP/1.1 301 Moved Permanently\r\n";
	headers += "Location: "	+ webFilePath + "\r\n";
	headers += "Content-Type: text/html;\r\n";
	headers += "Content-Length: 0\r\n";
	headers += "Connection: close";
	headers += "\r\n\r\n";
	clientPTR->responseString = headers;

	clientPTR->status = SEND_RESPONSE;
}

/* Ensures we do not read from error pages without going through poll*/
int ResponseHandler::openResponseFile(clientInfo *clientPTR, std::string filePath)
{
	setExtension(clientPTR); // is this a good place for this...?

	clientPTR->responseFileFd = open(filePath.c_str(), O_RDONLY);
	if (clientPTR->responseFileFd == -1)
	{
		std::cerr << RED << "\nopen() of response file failed:\n" << RESET << std::strerror(errno) << "\n\n";
		openErrorResponseFile(clientPTR);
	}
	else if (fcntl(clientPTR->responseFileFd, F_SETFL, O_NONBLOCK) == -1) // make file fd non-blocking
	{
		std::cerr << RED << "\nfcntl() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		setResponseCode(500);
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
		setResponseCode(500);
		openErrorResponseFile(clientPTR);
		return (-1); // Might not be needed
	}
	else if (fcntl(clientPTR->pipeToCgi[1], F_SETFL, O_NONBLOCK) == -1
	|| fcntl(clientPTR->pipeFromCgi[0], F_SETFL, O_NONBLOCK) == -1) // NOTE: We don't need to make the child process FDs non-blocking!
	{
		std::cerr << RED << "\nfcntl() in CGI failed:\n" << RESET << std::strerror(errno) << "\n\n";
		setResponseCode(500);
		openErrorResponseFile(clientPTR);
		return (-1); // Might not be needed
	}
	else
		clientPTR->status = EXECUTE_CGI;

	return (0);
}

/* Checks if files exist, what type they are and that their permissions are okay */
void ResponseHandler::checkFile(clientInfo *clientPTR)
{
	// we need to have separate variables for the local path vs the web path so redirects work the way they should.
	std::string filePath = clientPTR->parsedRequest.filePath;
	std::string defaultFilePath;
	std::string webFilePath = filePath;
	enum dirListStates dirListing {UNSET};
	std::string port = clientPTR->relatedServer->serverConfig->getPort();
	filePath = clientPTR->relatedServer->serverConfig->getRoot("/") + filePath;
	
	if (std::filesystem::is_directory(filePath))
	{
		if (filePath.back() == '/')
			defaultFilePath = filePath + clientPTR->relatedServer->serverConfig->getIndex();
		else
		{
			webFilePath += "/";
			buildRedirectResponse(webFilePath, clientPTR);
			return ;
		}
		if (std::filesystem::exists(defaultFilePath))
			filePath = defaultFilePath;
		else
		{
			dirListing = clientPTR->relatedServer->serverConfig->getDirListing(webFilePath);
			if (dirListing == TRUE)
				buildDirListingResponse(filePath, clientPTR);
			else
			{
				setResponseCode(403);
				openErrorResponseFile(clientPTR);
			}
			return ;
		}
	}

	std::ifstream ourFile(filePath);
	if (!ourFile)
	{
		webservLog.webservLog(ERROR, strerror(errno), true);
		if (errno == 2) // file missing
		{
			setResponseCode(404);
		}
		else if (errno == 13) // bad permissions
			setResponseCode(403);
		else
			setResponseCode(500);
		openErrorResponseFile(clientPTR);
		return ;
	}
	// CGI check & handling	
	if (clientPTR->parsedRequest.isCgi)
	{
		m_cgiHandler = new CgiHandler(clientPTR);

		openCgiPipes(clientPTR);
		
		return ;
	}

	// If everything went well, we start to build a aappropriate response
	openResponseFile(clientPTR, filePath);

}

/* Checks if the http request type is permitted by server configuration */
bool ResponseHandler::checkRequestAllowed(clientInfo *clientPTR)
{
	std::string permittedMethods = clientPTR->relatedServer->serverConfig->getMethods(clientPTR->parsedRequest.filePath);

	if (permittedMethods.find(requestTypeAsString) != std::string::npos)
		return true;
	else
		return false;
}

void ResponseHandler::handleRequest(clientInfo *clientPTR)
{
	
	switch (requestType)
	{
		case GET:
		{
			requestTypeAsString = "GET";
			if (checkRequestAllowed(clientPTR))
				checkFile(clientPTR);
			else
			{
				setResponseCode(405);
				openErrorResponseFile(clientPTR);
			}
			break;
		}	
		case POST:
		{
			requestTypeAsString = "POST";
			if (checkRequestAllowed(clientPTR))
			{
				if (clientPTR->reqType != MULTIPART) // DO we have to handle other types of file uploads...?
				{
					if (clientPTR->parsedRequest.cgiType != NONE) // check for CGI
						checkFile(clientPTR);
					else
					{
						setResponseCode(400); // Sort of a temp solution, what should we actually do if someone uses POST to do something else than CGI or File upload?
						openErrorResponseFile(clientPTR);
					}
					break ;
				}

				if (checkForMultipartFileData(clientPTR)) // Can you upload files some other way than using a HTML form...?
					prepareUploadFile(clientPTR);
				else
					openErrorResponseFile(clientPTR);

				break ;
			}
			else
			{
				setResponseCode(405);
				openErrorResponseFile(clientPTR);
			}
			break ;
		}
		case DELETE:
		{
			requestTypeAsString = "DELETE";
			if (checkRequestAllowed(clientPTR))
				deleteHandler(clientPTR, clientPTR->parsedRequest.filePath);
			else
			{
				setResponseCode(405);
				openErrorResponseFile(clientPTR);
			}
			break ;
			
		}
		default:
			webservLog.webservLog(ERROR, "unhandled parseRequest", true);
	}

}

/* Returns the end index of Content-Disposition -header line related to an uploaded file. (so returned index is in the "\r\n" sequence)*/
bool	ResponseHandler::checkForMultipartFileData(clientInfo *clientPTR)
{
	std::string temp = "boundary=";
	size_t boundaryStartIdx = clientPTR->requestString.find(temp);
	if (boundaryStartIdx == std::string::npos)
	{
		std::cerr << RED << "\ncheckForMultipartFileData() failed:\n" << RESET << "Boundary not found\n\n";
		setResponseCode(400);
		return false;
	}
	boundaryStartIdx += temp.length();
	size_t boundaryEndIdx = clientPTR->requestString.find("\r\n", boundaryStartIdx);
	if (boundaryEndIdx == std::string::npos)
	{
		std::cerr << RED << "\ncheckForMultipartFileData() failed:\n" << RESET << "Boundary header has no ending\n\n";
		setResponseCode(400);
		return false;
	}
	std::string boundaryStr = clientPTR->requestString.substr(boundaryStartIdx, boundaryEndIdx - boundaryStartIdx);
	clientPTR->multipartBoundaryStr = boundaryStr;

	std::string endBoundary = boundaryStr + "--";
	size_t startIdx = 0;
	while (1)
	{
		startIdx = clientPTR->requestString.find(boundaryStr, startIdx);
		if (startIdx == std::string::npos)
		{
			std::cerr << RED << "\ncheckForMultipartFileData() failed:\n" << RESET << "Boundary not found\n\n";
			setResponseCode(400);
			return false;
		}
		std::string lineStr = clientPTR->requestString.substr(startIdx, endBoundary.size());
		if (lineStr == endBoundary)
			break ;
		
		startIdx += boundaryStr.size() + 2; // +2 to skip the \r\n sequence
		size_t endIdx = clientPTR->requestString.find("\r\n", startIdx);
		if (endIdx == std::string::npos)
		{
			std::cerr << RED << "\ncheckForMultipartFileData() failed:\n" << RESET << "Boundary line has no ending\n\n";
			setResponseCode(400);
			return false;
		}
		lineStr = clientPTR->requestString.substr(startIdx, endIdx - startIdx);

		std::vector<std::string> tokensVec;
		std::istringstream stream(lineStr);
		std::string tempToken;

		while (std::getline(stream, tempToken, ';')) 
			tokensVec.push_back(tempToken);

		if (tokensVec.size() > 2 && tokensVec[2].find("filename=") != std::string::npos)
		{
			size_t nameStart = tokensVec[2].find_first_of('"') + 1;
			size_t nameEnd = tokensVec[2].find_last_of('"');

			clientPTR->uploadFileName = "home/images/uploads/" + tokensVec[2].substr(nameStart, nameEnd - nameStart); // HARD CODED for now
			clientPTR->multipartFileDataStartIdx = clientPTR->requestString.find("\r\n\r\n", endIdx) + 4;
			return (true);
		}
	}

	std::cerr << RED << "\ncheckForMultipartFileData() failed:\n" << RESET << "No files found in the form upload request body\n\n";
	setResponseCode(400);
	return (false);
}

void	ResponseHandler::prepareUploadFile(clientInfo *clientPTR)
{
	if (std::filesystem::exists(clientPTR->uploadFileName))
	{
		std::cerr << RED << "\nFile upload failed:\n" << RESET << "File with the same name already exists.\n\n";
		setResponseCode(400); // is this ok...?
		openErrorResponseFile(clientPTR);
		return ;
	}

	//	checkExtension(filePath);  --> Is this needed here...?! Has been commented out for a long time -Panu

	clientPTR->uploadFileFd = open(clientPTR->uploadFileName.c_str(), O_RDWR | O_CREAT, 0644);
	if (clientPTR->uploadFileFd == -1)
	{
		std::cerr << RED << "\nopen() of upload file failed:\n" << RESET << std::strerror(errno) << "\n\n";
		if (errno == 13) // bad permissions --> is this possible in theory?
			setResponseCode(403);
		else
			setResponseCode(500);
		openErrorResponseFile(clientPTR);
	}
	else if (fcntl(clientPTR->uploadFileFd, F_SETFL, O_NONBLOCK) == -1) // make file fd non-blocking
	{
		std::cerr << RED << "\nfcntl() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		setResponseCode(500);
		openErrorResponseFile(clientPTR);
	}
	else
		clientPTR->status = SAVE_FILE;

}

void ResponseHandler::deleteHandler(clientInfo *clientPTR, std::string filePath)
{
	(void)clientPTR;
	(void)filePath;
	
	std::string serverLocalPath = clientPTR->relatedServer->serverConfig->getRoot("/") + filePath;
	if (std::filesystem::is_directory(serverLocalPath))
	{
		if (std::filesystem::is_empty(serverLocalPath))
		{
			try{
				std::filesystem::remove(serverLocalPath);
				setResponseCode(204);
				build204Response(clientPTR);
			}
			catch(std::exception& e)
			{
				std::cerr << "Attempt to remove " << serverLocalPath << " failed: " << e.what() << std::endl;
			}
		}
		else
		{
			setResponseCode(400); //We can't remove directories that are not empty with delete method
			openErrorResponseFile(clientPTR);
		}
	}
	else
	{
		if (std::filesystem::exists(serverLocalPath))
		{
			auto perms = std::filesystem::status(serverLocalPath).permissions();
			if ((perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none) // Why were these originally group_write...? I'm testing owner_write for now -Panu
			{
				try{
					std::filesystem::remove(serverLocalPath);
					setResponseCode(204);
					build204Response(clientPTR);
					}
				
				catch(std::exception& e)
				{
					webservLog.webservLog( ERROR, "Attempt to remove " + serverLocalPath + " failed: " + e.what(), true);
				}
			}
			else
			{
				setResponseCode(403);
				openErrorResponseFile(clientPTR);
			}

		}
		else
		{
			setResponseCode(404);
			openErrorResponseFile(clientPTR);
		}
	}
}

void ResponseHandler::build204Response(clientInfo *clientPTR)
{
	std::string	headers;
	headers = "HTTP/1.1 204 No Content\r\n";
	headers += "Server: 42 webserv\r\n";
	headers += "Content-Length: 0\r\n";
	clientPTR->responseString = headers;
	clientPTR->status = SEND_RESPONSE;
}

/* Builds successful responses */
int ResponseHandler::buildResponse(clientInfo *clientPTR)
{
	if (clientPTR->responseFileFd == -1)
	{
		webservLog.webservLog(ERROR, "BuildResponse called before response file was opened", true);
		return (-1); // Check this later
	}

	char 	buffer[100024];
	int		bytesRead;
	int		readPerCall = 100023;

	bytesRead = read(clientPTR->responseFileFd, buffer, readPerCall);
	if (bytesRead == -1)
	{
		std::string errorInfo = std::strerror(errno);
		webservLog.webservLog(ERROR, "read() of response page file failed: " + errorInfo, true);
		setResponseCode(500); // is this ok?
		openErrorResponseFile(clientPTR);
		return (-1);
	}

	buffer[bytesRead] = '\0';
	clientPTR->responseBody.append(buffer, bytesRead);

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
	}
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

/* Builds 500 responses directly so we don't have to use poll to read error file */
void ResponseHandler::build500Response(clientInfo *clientPTR)
{
	std::string	headers;
	std::string content;

	content = "<html lang='EN'>"
	"<head><title>500 Internal Server Error</title></head>"
		"<body>"
			"<h1>500 Internal Server Error</h1>"
			"<p>Something went wrong.</p>"
		"</body>"
	"</html>";
	headers = "HTTP/1.1 " + std::to_string(500) + " Internal Server Error\r\n";
	headers += "Content-Type: text/html;\r\n";
	headers += "Content-Length: ";
	headers += std::to_string(content.length());
	headers += "\r\n\r\n";
	clientPTR->responseString = headers + content;
	webservLog.webservLog(INFO, "responseString: " + clientPTR->responseString, false);
}


bool ResponseHandler::isValidErrorFile(std::string &errorFileName)
{
	if (errorFileName == "") // not found in config
		return false;

	if (errorFileName[0] == '/') // we need to strip the leading / so it uses relative paths instead of absolute ones
		errorFileName = errorFileName.substr(1, errorFileName.size() - 1);

	if (!std::filesystem::exists(errorFileName)) // non-existing
		return false;

	auto perms = std::filesystem::status(errorFileName).permissions();
	if ((perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none) // no permissions
		return false;

	return true;
}

void ResponseHandler::openErrorResponseFile(clientInfo *clientPTR)
{
	std::string errorFileName = clientPTR->relatedServer->serverConfig->getErrorPages(responseCode);

	if (!isValidErrorFile(errorFileName))
	{
		errorFileName = clientPTR->relatedServer->serverConfig->getDefaultErrorPages(responseCode);
		if (!isValidErrorFile(errorFileName))
		{
			webservLog.webservLog(ERROR, "openErrorResponseFile() failed: Could not locate proper error response file", false);
			build500Response(clientPTR);
			clientPTR->status = SEND_RESPONSE;
			return ;
		}
	}

	clientPTR->errorFileFd = open(errorFileName.c_str(), O_RDONLY);
	if (clientPTR->errorFileFd == -1)
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "\nopen() of error page file failed:\n" + errorString + "\n\n", true);
		build500Response(clientPTR);
		clientPTR->status = SEND_RESPONSE;
	}
	else if (fcntl(clientPTR->errorFileFd, F_SETFL, O_NONBLOCK) == -1) // make file fd non-blocking
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "\nfcntl() failed:\n" + errorString + "\n\n", true);
		build500Response(clientPTR);
		clientPTR->status = SEND_RESPONSE;
	}
	else
		clientPTR->status = BUILD_ERRORPAGE;
}


void ResponseHandler::buildErrorResponse(clientInfo *clientPTR)
{
	if (clientPTR->errorFileFd == -1)
		webservLog.webservLog(ERROR, "\nbuildErrorResponse called before error file was opened\n", true);
	//need to update this based on config file path to error pages
	//tested with this and it works. now just fetch this from the configuration data. --- Patrik

	char 	buffer[100024];
	int		bytesRead;
	int		readPerCall = 100023;

	bytesRead = read(clientPTR->errorFileFd, buffer, readPerCall);
	if (bytesRead == -1)
	{
		std::cerr << RED << "\nread() of error page file failed:\n" << RESET << std::strerror(errno) << "\n\n";
		setResponseCode(500);
		openErrorResponseFile(clientPTR);
		return ;
	}

	buffer[bytesRead] = '\0';
	clientPTR->responseBody += buffer;

	if (bytesRead < readPerCall)
	{
		contentType = "text/html";
		std::string	headers;

		headers = "HTTP/1.1 " + std::to_string(getResponseCode()) + " " + errorCodes.at(getResponseCode()) + "\r\n"; // Error handling fot .at() method!
		headers += "Content-Type: " + contentType + "\r\n";
		headers += "Content-Length: ";
		headers += std::to_string(clientPTR->responseBody.length());
		headers += "\r\n\r\n";
		clientPTR->responseString = headers + clientPTR->responseBody;
		webservLog.webservLog(INFO, "responseString: " + clientPTR->responseString, false);
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
