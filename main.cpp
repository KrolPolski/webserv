#include "ConnectionHandler.hpp"
#include "ResponseHandler.hpp"

int main(int argc, char *argv[])
{
	if (argc != 2 || argv == NULL)
	{
		/* Let's activate this once the configuration file parsing works

		std::cerr << RED << "\nInvalid arguments! Please provide only one configuration file.\n\n" << RESET;
		return (1);
		*/
	}

	ConnectionHandler	handler;
	unsigned int		portArr[1]; // Just a test
	int					portArrSize = 1; // Just a test
	//ResponseHandler		responseHandler;

	/*
		Patrik:

		Your configuration file parsing happens here OR in the beginning of the initServer -function (before the loop).
		Parse all the necessary info and save it in some kind of struct-array. Then we figure out a way to pass
		that info struct array to my ConnectionHandler.
	*/

	portArr[0] = 8080;

	if (handler.initServers(portArr, portArrSize) == -1)
		return (1);

	if (handler.startServers() == -1)
		return (1);

	return (0);
}