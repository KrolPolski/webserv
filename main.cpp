#include "ResponseHandler.hpp"
#include "ConnectionHandler.hpp"
#include "Logger.hpp"

/*
	MAIN FUNCTION
*/
Logger webservLog("logfile.log");

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		std::cerr << RED << "\nInvalid arguments! Please provide only one configuration file.\n\n" << RESET;
		return (1);
	}

	signal(SIGINT, sigIntHandler); // is this needed...?

	ConnectionHandler	handler;
	char	configFileName[] = "configurations/test.conf"; // our default config file name here
	char	*configPTR = configFileName;

	if (argc == 2)
		configPTR = argv[1];
	
	if (handler.initServers(configPTR) == -1)
		return (1);

	webservLog.webservLog(INFO, "Starting server", true);
	if (handler.startServers() == -1)
		return (1);

	return (0);
}
