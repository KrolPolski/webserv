#include "ResponseHandler.hpp"
#include "ConnectionHandler.hpp"

int main(int argc, char *argv[])
{
	if (argc != 2 || argv == NULL)
	{
		std::cerr << RED << "\nInvalid arguments! Please provide only one configuration file.\n\n" << RESET;
		return (1);
	}

	ConnectionHandler	handler;

	if (handler.initServers(argv[1]) == -1)
		return (1);

	if (handler.startServers() == -1)
		return (1);

	return (0);
}