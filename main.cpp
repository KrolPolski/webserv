#include "ResponseHandler.hpp"
#include "ConnectionHandler.hpp"

/*
	MAIN FUNCTION
*/

int main(int argc, char *argv[])
{
	if (argc != 2 || argv == NULL)
	{
		/* Let's activate this once the configuration file parsing works

		std::cerr << RED << "\nInvalid arguments! Please provide only one configuration file.\n\n" << RESET;
		return (1);
		*/
	}

	signal(SIGINT, sigIntHandler); // is this needed...?

	ConnectionHandler	handler;
	unsigned int		portArr[1]; // Just a test
	int					portArrSize = 1; // Just a test
	//ResponseHandler		responseHandler;

	portArr[0] = 8080;

	if (handler.initServers(portArr, portArrSize) == -1)
		return (1);

	if (handler.startServers() == -1)
		return (1);

	return (0);
}
