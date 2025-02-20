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
	{408, "Client Timeout"},
  {409, "Conflict"},
	{411, "Length Required"},
	{413, "Payload Too Large"},
	{414, "URI Too Long"},
	{431, "Request Header Fields Too Large"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{505, "HTTP Version Not Supported"}
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

/* Identifies the data type of the request by extension so we can use the appropriate headers in our responses */
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
void ResponseHandler::buildRedirectResponse301(std::string webFilePath, clientInfo *clientPTR)
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

/* Builds a 307 response header when required for redirects in locations*/
void ResponseHandler::buildRedirectResponse307(std::string webFilePath, clientInfo *clientPTR)
{
	setResponseCode(clientPTR->relatedServer->serverConfig->getRedirectStatusCode(webFilePath));
	std::string headers;
	headers = "HTTP/1.1 307 Temporary Redirect\r\n";
	headers += "Location: "	+ clientPTR->relatedServer->serverConfig->getRedirectLocation(webFilePath) + "\r\n";
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
	setExtension(clientPTR);

	clientPTR->responseFileFd = open(filePath.c_str(), O_RDONLY);
	if (clientPTR->responseFileFd == -1)
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "open() of response file failed: " + errorString, true);
		openErrorResponseFile(clientPTR);
	}
	else if (fcntl(clientPTR->responseFileFd, F_SETFL, O_NONBLOCK) == -1) // make file fd non-blocking
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "fcntl() failed: " + errorString, true);
		setResponseCode(500);
		openErrorResponseFile(clientPTR);
	}
	else
		clientPTR->status = BUILD_RESPONSE;
	return (0);
}

int ResponseHandler::openCgiPipes(clientInfo *clientPTR)
{
	if (pipe(clientPTR->pipeToCgi) == -1 || pipe(clientPTR->pipeFromCgi) == -1)
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "pipe() in CGI failed: " + errorString, true);
		setResponseCode(500);
		openErrorResponseFile(clientPTR);
		return (-1);
	}
	else if (fcntl(clientPTR->pipeToCgi[1], F_SETFL, O_NONBLOCK) == -1
	|| fcntl(clientPTR->pipeFromCgi[0], F_SETFL, O_NONBLOCK) == -1) 
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "fcntl() in CGI failed: " + errorString, true);
		setResponseCode(500);
		openErrorResponseFile(clientPTR);
		return (-1);
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
	filePath = clientPTR->relatedServer->serverConfig->getRoot("/") + filePath;
  
	if (std::filesystem::exists(filePath) && checkRightsOfDirectory(filePath, clientPTR) == false)
		return ;
	if (clientPTR->relatedServer->serverConfig->isRedirectSet(webFilePath) == true)
	{
		buildRedirectResponse307(webFilePath, clientPTR);
		return ;
	}
	if (std::filesystem::is_directory(filePath))
	{
		if (filePath.back() == '/')
			defaultFilePath = filePath + clientPTR->relatedServer->serverConfig->getIndex();
		else
		{
			webFilePath += "/";
			buildRedirectResponse301(webFilePath, clientPTR);
			return ;
		}
		if (clientPTR->relatedServer->serverConfig->isLocationConfigured(webFilePath) == false)
		{
			setResponseCode(404);
			openErrorResponseFile(clientPTR);
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

bool ResponseHandler::checkRightsOfDirectory(std::string directoryPath, clientInfo *clientPTR)
{
	struct stat info;

	if (stat(directoryPath.c_str(), &info) == 0 && S_ISDIR(info.st_mode))
	{
		if (!(info.st_mode & S_IRUSR) || !(info.st_mode & S_IWUSR) || !(info.st_mode & S_IXUSR))
		{
			setResponseCode(403);
			openErrorResponseFile(clientPTR);
			return false ;
		}
		if (!(info.st_mode & S_IROTH) || !(info.st_mode & S_IXOTH))
		{
			setResponseCode(403);
			openErrorResponseFile(clientPTR);
			return false ;
		}
		if (!(info.st_mode & S_IRGRP) || !(info.st_mode & S_IXGRP))
		{
			setResponseCode(403);
			openErrorResponseFile(clientPTR);
			return false ;
		}
		return true;
	}
	else if (stat(directoryPath.c_str(), &info) == 0 && S_ISREG(info.st_mode))
		return true;
	else
	{
		webservLog.webservLog(ERROR, strerror(errno), true);
		if (errno == 2) // file missing
			setResponseCode(404);
		else if (errno == 13) // bad permissions
			setResponseCode(403);
		else if (errno == 20) // not a directory
			setResponseCode(400);
		else
			setResponseCode(500);
		openErrorResponseFile(clientPTR);
		return false;
	}
}

void ResponseHandler::handleRequest(clientInfo *clientPTR)
{
	std::string filePath = clientPTR->relatedServer->serverConfig->getRoot("/") + clientPTR->parsedRequest.filePath;
	switch (requestType)
	{
		case GET:
		{
			requestTypeAsString = "GET";
			if (!std::filesystem::exists(filePath) && clientPTR->relatedServer->serverConfig->isRedirectSet(clientPTR->parsedRequest.filePath) == true)
			{
				if (checkRequestAllowed(clientPTR))
					checkFile(clientPTR);
				else
				{
					setResponseCode(405);
					openErrorResponseFile(clientPTR);
				}
			}
			else if (std::filesystem::exists(filePath))
			{
				if (checkRequestAllowed(clientPTR))
					checkFile(clientPTR);
				else
				{
					setResponseCode(405);
					openErrorResponseFile(clientPTR);
				}
			}
			else
			{
				setResponseCode(404);
				openErrorResponseFile(clientPTR);
			}
			break;
		}	
		case POST:
		{
			requestTypeAsString = "POST";
			if (std::filesystem::exists(filePath))
			{
				if (checkRequestAllowed(clientPTR))
				{
					if (clientPTR->reqType != MULTIPART)
					{
						if (clientPTR->parsedRequest.cgiType != NONE) // check for CGI
							checkFile(clientPTR);
						else
						{
							setResponseCode(400); // 405?
							openErrorResponseFile(clientPTR);
						}
						break ;
					}
					if (checkForMultipartFileData(clientPTR))
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
			}
			else
			{
				setResponseCode(404);
				openErrorResponseFile(clientPTR);
			}
			break ;
		}
		case DELETE:
		{
			requestTypeAsString = "DELETE";
			if (std::filesystem::exists(filePath))
			{
				if (checkRequestAllowed(clientPTR))
					deleteHandler(clientPTR, clientPTR->parsedRequest.filePath);
				else
				{
					setResponseCode(405);
					openErrorResponseFile(clientPTR);
				}
			}
			else
			{
				setResponseCode(404);
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
		webservLog.webservLog(ERROR, "checkForMultipartFileData() failed: Boundary not found", true);
		setResponseCode(400);
		return false;
	}
	boundaryStartIdx += temp.length();
	size_t boundaryEndIdx = clientPTR->requestString.find("\r\n", boundaryStartIdx);
	if (boundaryEndIdx == std::string::npos)
	{
		webservLog.webservLog(ERROR, "checkForMultipartFileData() failed: Boundary header has no ending", true);
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
			webservLog.webservLog(ERROR, "checkForMultipartFileData() failed: Boundary not found", true);
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
			webservLog.webservLog(ERROR, "checkForMultipartFileData() failed: Boundary line has no ending", true);
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

			std::string	uploadDirPath;
			if (clientPTR->relatedServer->serverConfig->isUploadDirSet(clientPTR->parsedRequest.filePath) == true)
			{
				uploadDirPath = clientPTR->relatedServer->serverConfig->getUploadDir(clientPTR->parsedRequest.filePath);
				struct stat info;
				if (stat(uploadDirPath.c_str(), &info) == 0 && S_ISDIR(info.st_mode))
				{
					if (!(info.st_mode & S_IRUSR) || !(info.st_mode & S_IWUSR)
					|| !(info.st_mode & S_IXUSR) || !(info.st_mode & S_IROTH)
					|| !(info.st_mode & S_IXOTH) || !(info.st_mode & S_IRGRP)
					|| !(info.st_mode & S_IXGRP))
					{
						uploadDirPath = clientPTR->relatedServer->serverConfig->getRoot(clientPTR->parsedRequest.filePath) + "/";
					}
				}
			}
			else
				uploadDirPath = clientPTR->relatedServer->serverConfig->getRoot(clientPTR->parsedRequest.filePath) + "/";
			clientPTR->uploadWebPath = "http://localhost:" + clientPTR->relatedServer->serverConfig->getPort() + "/" + uploadDirPath + tokensVec[2].substr(nameStart, nameEnd - nameStart);// HARD CODED for now
			clientPTR->uploadFileName = uploadDirPath + tokensVec[2].substr(nameStart, nameEnd - nameStart);
			clientPTR->multipartFileDataStartIdx = clientPTR->requestString.find("\r\n\r\n", endIdx) + 4;
			return (true);
		}
	}

	webservLog.webservLog(ERROR, "checkForMultipartFileData() failed: No files found in the form upload request body", true);
	setResponseCode(400);
	return (false);
}

void	ResponseHandler::prepareUploadFile(clientInfo *clientPTR)
{
	if (std::filesystem::exists(clientPTR->uploadFileName))
	{
		webservLog.webservLog(ERROR, "File upload failed: File with the same name already exists.", true);
		setResponseCode(409);
		openErrorResponseFile(clientPTR);
		return ;
	}
	clientPTR->uploadFileFd = open(clientPTR->uploadFileName.c_str(), O_RDWR | O_CREAT, 0644);
	if (clientPTR->uploadFileFd == -1)
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "open() of upload file failed: " + errorString, true);
		if (errno == 13) // bad permissions
			setResponseCode(403);
		else
			setResponseCode(500);
		openErrorResponseFile(clientPTR);
	}
	else if (fcntl(clientPTR->uploadFileFd, F_SETFL, O_NONBLOCK) == -1) // make file fd non-blocking
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "fcntl() failed: " + errorString, true);
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
				webservLog.webservLog(ERROR, "Attempt to remove " + serverLocalPath + " failed: " + e.what(), true);
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
			if ((perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none)
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
		return (-1);
	}

	char 	buffer[100024];
	int		bytesRead;
	int		readPerCall = 100023;

	bytesRead = read(clientPTR->responseFileFd, buffer, readPerCall);
	if (bytesRead == -1)
	{
		std::string errorInfo = std::strerror(errno);
		webservLog.webservLog(ERROR, "read() of response page file failed: " + errorInfo, true);
		setResponseCode(500);
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

/* Builds successful upload HTTP response */
void ResponseHandler::build201Response(clientInfo *clientPTR, std::string webPathToFile)
{
	std::string headers;
	headers = "HTTP/1.1 201 Created\r\n";
	headers += "Server: 42 webserv\r\n";
	headers += "Location: " + webPathToFile + "\r\n";
	headers += "Content-Type: application/json\r\n";
	clientPTR->responseBody = R"({"message": "File uploaded successfully",
	"url": )" + webPathToFile + "}";
	headers += "Content-Length: " + std::to_string(clientPTR->responseBody.length()) + "\r\n\r\n";
	std::cout << headers << clientPTR->responseBody << std::endl;
	clientPTR->responseString = headers + clientPTR->responseBody;
	clientPTR->status = SEND_RESPONSE;
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
		webservLog.webservLog(ERROR, "open() of error page file failed: " + errorString, true);
		build500Response(clientPTR);
		clientPTR->status = SEND_RESPONSE;
	}
	else if (fcntl(clientPTR->errorFileFd, F_SETFL, O_NONBLOCK) == -1) // make file fd non-blocking
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "fcntl() failed: " + errorString, true);
		build500Response(clientPTR);
		clientPTR->status = SEND_RESPONSE;
	}
	else
		clientPTR->status = BUILD_ERRORPAGE;
}


void ResponseHandler::buildErrorResponse(clientInfo *clientPTR)
{
	if (clientPTR->errorFileFd == -1)
		webservLog.webservLog(ERROR, "buildErrorResponse called before error file was opened", true);

	char 	buffer[100024];
	int		bytesRead;
	int		readPerCall = 100023;

	bytesRead = read(clientPTR->errorFileFd, buffer, readPerCall);
	if (bytesRead == -1)
	{
		std::string errorString = std::strerror(errno);
		webservLog.webservLog(ERROR, "read() of error page file failed: " + errorString, true);
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
