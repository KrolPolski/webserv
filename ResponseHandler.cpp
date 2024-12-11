#include "ResponseHandler.hpp"
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

void ResponseHandler::checkRequestType(clientInfo *ClientPTR, std::string requestString)
{
    std::cout << "We managed to get to checkRequestType" << std::endl;
    std::cout << requestString << std::endl;
	std::cout << "We should have printed the http request by now" << std::endl;
	if (!requestString.compare(0, 3, "GET"))
	{
		std::cout << "GET request detected woo" << std::endl;
		setRequestType(GET);
	}
	else if (!requestString.compare(0, 4, "POST"))
	{
		std::cout << "POST request detected woo" << std::endl;
		setRequestType(POST);
	}
	else if (!requestString.compare(0, 6, "DELETE"))
	{
		std::cout << "DELETE request detected woo" << std::endl;
		setRequestType(DELETE);
	}
	else
	{
		std::cout << "INVALID request detected woo" << std::endl;
		setRequestType(INVALID);
		setResponseCode(400);
	}
}

void ResponseHandler::checkExtension(std::string filePath)
{
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
}

int ResponseHandler::checkFile(clientInfo *clientPTR, std::string filePath)
{
    std::cout << "We should now check if the file exists" << std::endl;
	// this has to be fixed once we have a configuration parsing appropriately
	if (filePath == "/")
		filePath = "home/index.html";
	else
		filePath = "home" + filePath;
	std::string content;
	std::ifstream ourFile(filePath);
	/*We need to detect which type of error we got with the file, so we 
	can send the appropriate response code*/
	if (!ourFile)
	{
		std::cerr << "Error: " << strerror(errno) << errno << std::endl;
		if (errno == 2) //file missing
			setResponseCode(404);
		else if (errno == 13) //bad permissions
			setResponseCode(403);
		else
			setResponseCode(500);
		std::cerr << "Response Code: " << getResponseCode() << std::endl;
		return -1;
	}
	
	std::string line;
	while (std::getline(ourFile, line))
		content += line + "\n";
	checkExtension(filePath);
	
	std::string	headers;

	headers = "HTTP/1.1 200 OK\r\n";
	headers += "Content-Type: " + contentType + "\r\n";
	headers += "Content-Length: ";
	headers += std::to_string(content.length());
	headers += "\r\n\r\n";
	clientPTR->responseString = headers + content;
	std::cout << "responseString: " << clientPTR->responseString << std::endl;
	ourFile.close();
	setResponseCode(200);
	std::cout << "Must have opened the file with no errors" << std::endl;
	return (0);
}

void ResponseHandler::parseRequest(clientInfo *clientPTR, std::string requestString)
{
	switch (requestType)
	{
		case GET:
		{
			std::istringstream stream(requestString);
			std::vector<std::string> reqVec;
			std::string line;
			std::cout <<"\nBeginning line splits" << std::endl;
			while (std::getline(stream, line, '\n'))
			{
				reqVec.push_back(line);
				std::cout << line << std::endl;
			}
			std::cout << "Line splits done" << std::endl;
			std::cout << "Line one is: " << reqVec.at(0) << std::endl;
			std::istringstream streamL1(reqVec.at(0));
			std::string phrase; 
			std::vector<std::string> lineOne;
			std::cout << "\nBeginning phrase split" << std::endl;
			while (std::getline(streamL1, phrase, ' '))
			{
				lineOne.push_back(phrase);
				std::cout << phrase << std::endl;
			}
			if (lineOne.size() >= 2)
				checkFile(clientPTR, lineOne.at(1));
			break;
		}	
		default:
			std::cout << "unhandled parseRequest" << std::endl;
	}
}

const enum requestTypes& ResponseHandler::getRequestType() const
{
	return requestType;
}

const unsigned int& ResponseHandler::getResponseCode() const
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

}