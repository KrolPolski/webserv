#include "ResponseHandler.hpp"
#include "CgiHandler.hpp"
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
		std::cout << "GET request detected woo" << std::endl;
		setRequestType(GET);
	}
	else if (!requestString.compare(0, 5, "POST "))
	{
		std::cout << "POST request detected woo" << std::endl;
		setRequestType(POST);
	}
	else if (!requestString.compare(0, 7, "DELETE "))
	{
		std::cout << "DELETE request detected woo" << std::endl;
		setRequestType(DELETE);
	}
	else
	{
		std::cout << "INVALID request detected woo" << std::endl;
		setRequestType(INVALID);
		setResponseCode(400);
		buildErrorResponse(ClientPTR);
	}
}

void ResponseHandler::checkExtension(std::string filePath)
{
	std::cout << "Inside checkExtension" << std::endl;
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

	std::cout << "filePath: " << filePath << " Extension: " << extension << " Type: " << extensionTypes.at(extension) << std::endl;
	try
	{
		contentType = extensionTypes.at(extension);
		std::cout << "contentType: " << contentType << std::endl;
	} 
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		//there's probably a response code for this
	}
	std::cout << "exiting checkExtension" << std::endl;
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
//	std::cout << "responseString: " << clientPTR->responseString << std::endl;
}
int ResponseHandler::checkFile(clientInfo *clientPTR, std::string filePath)
{
  
//    std::cout << "We should now check if the file exists" << std::endl;
	// we need to have separate variables for the local path vs the web path so redirects work the way they should.
	std::string defaultFilePath;
	std::string webFilePath;

	filePath = clientPTR->parsedRequest.filePath;
	webFilePath = filePath;
	std::string port = clientPTR->relatedServer->serverConfig->getPort();
	std::cout << "Port returned: " << port << std::endl;
	filePath = clientPTR->relatedServer->serverConfig->getRoot("/") + filePath;
	std::cout << "Updated file path is: " << filePath << std::endl;
	
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
			std::cout <<"Call Patrik's function for directory listing" << std::endl; // DirListing needs to be checked for if ON/OFF ---- Patrik
			buildDirListingResponse(filePath, clientPTR);
			return 0;
		}
	}
	// If we get here then we have concluded that it isn't a directory.
	
	std::ifstream ourFile(filePath);
	/*We need to detect which type of error we got with the file, so we 
	can send the appropriate response code*/
	if (!ourFile)
	{
		std::cerr << "Error: " << strerror(errno) << errno << std::endl;
		if (errno == 2) //file missing
		{
			setResponseCode(404);
		}
		else if (errno == 13) //bad permissions
			setResponseCode(403);
		else
			setResponseCode(500);
		std::cerr << "Response Code: " << getResponseCode() << std::endl;
		buildErrorResponse(clientPTR);
		return -1;
	}

	/*
		Panu addition (CGI)
	*/

	if (clientPTR->parsedRequest.isCgi)
	{
		// This needs to be saved in the connection handler somehow... So we can use it in the poll() loop

		CgiHandler	cgiHandler(*clientPTR);

		if (cgiHandler.executeCgi() == -1)
			return (-1);
		
  		setResponseCode(200);
		return (0);
	}

	/*
		Addition end
	*/
	std::cout << "We are about to checkExtension" << std::endl;
	std::string line;
	while (std::getline(ourFile, line))
		content += line + "\n";
	checkExtension(filePath);
	std::cout << "We are done checking Extension" << std::endl;
	std::string	headers;
	setResponseCode(200);
	headers = "HTTP/1.1 200 OK\r\n";
	headers += "Content-Type: " + contentType + "\r\n";
	headers += "Content-Length: ";
	headers += std::to_string(content.length());
	headers += "\r\n\r\n";
	clientPTR->responseString = headers + content;
//	std::cout << "responseString: " << clientPTR->responseString << std::endl;
	ourFile.close();
  
	std::cout << "Must have opened the file with no errors" << std::endl;
	return (0);
}

bool ResponseHandler::checkRequestAllowed(clientInfo *clientPTR, std::string filePath)
{
	(void)filePath;
	std::string permittedMethods = clientPTR->relatedServer->serverConfig->getMethods(filePath);
	std::cout << "We decided permitted Methods are: " << permittedMethods << std::endl;
	std::cout << "Our request type is " << requestTypeAsString << std::endl;
	if (permittedMethods.find(requestTypeAsString) != std::string::npos)
	{
		std::cout << "Method must have been permitted" << std::endl;
		return true;
	}
	else
	{
		std::cout << "Method not permitted, should show 405 page" << std::endl;
		responseCode = 405;
		return false;
	}
}
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
						{setResponseCode(405);
						buildErrorResponse(clientPTR);}
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

void ResponseHandler::buildErrorResponse(clientInfo *clientPTR)
{
	std::string content;
	std::cout << "We got to buildErrorResponse" << std::endl;
	//need to update this based on config file path to error pages
	//tested with this and it works. now just fetch this from the configuration data. --- Patrik
	std::string errorFileName = "home/error/" + std::to_string(getResponseCode()) + ".html";
	std::ifstream ourFile(errorFileName);
	std::string line;
	while (std::getline(ourFile, line))
		content += line + "\n";
	checkExtension(errorFileName);
	
	std::string	headers;

	headers = "HTTP/1.1 " + std::to_string(getResponseCode()) + " " + errorCodes.at(getResponseCode()) + "\r\n";
	headers += "Content-Type: " + contentType + "\r\n";
	headers += "Content-Length: ";
	headers += std::to_string(content.length());
	headers += "\r\n\r\n";
	clientPTR->responseString = headers + content;
	std::cout << "responseString: " << clientPTR->responseString << std::endl;
	ourFile.close();
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
}
