/* *************************************************************************** */
/*                                                                             */
/*  $$\      $$\           $$\                                                 */
/*  $$ | $\  $$ |          $$ |                                                */
/*  $$ |$$$\ $$ | $$$$$$\  $$$$$$$\   $$$$$$$\  $$$$$$\   $$$$$$\ $$\    $$\   */
/*  $$ $$ $$\$$ |$$  __$$\ $$  __$$\ $$  _____|$$  __$$\ $$  __$$\\$$\  $$  |  */
/*  $$$$  _$$$$ |$$$$$$$$ |$$ |  $$ |\$$$$$$\  $$$$$$$$ |$$ |  \__|\$$\$$  /   */
/*  $$$  / \$$$ |$$   ____|$$ |  $$ | \____$$\ $$   ____|$$ |       \$$$  /    */
/*  $$  /   \$$ |\$$$$$$$\ $$$$$$$  |$$$$$$$  |\$$$$$$$\ $$ |        \$  /     */
/*  \__/     \__| \_______|\_______/ \_______/  \_______|\__|         \_/      */
/*                                                                             */
/*   By: Panu Kangas, Ryan Boudwin, Patrik Lång                                */
/*                                                                             */
/* *************************************************************************** */

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
		webservLog.webservLog(ERROR, "Invalid arguments! Please provide only one configuration file.", true);
		return (1);
	}

	signal(SIGINT, sigIntHandler);

	ConnectionHandler	handler;
	char	configFileName[] = "configurations/complete.conf";
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
