#include "ConnectionHandler.hpp"

void	ConnectionHandler::getMethod(clientInfo *clientPTR)
{
	if (!clientPTR->requestString.compare(0, 4, "GET "))
		clientPTR->requestInfo.method = "GET";
	else if (!clientPTR->requestString.compare(0, 5, "POST "))
		clientPTR->requestInfo.method = "POST";
	else if (!clientPTR->requestString.compare(0, 7, "DELETE "))
		clientPTR->requestInfo.method = "DELETE";
	else
		clientPTR->requestInfo.method = "INVALID";
}

