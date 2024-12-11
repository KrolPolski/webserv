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

#include "ConfigurationHandler.hpp"

int	main(int argc, char **argv)
{
	if (argc == 2)
	{
		try
		{
			std::map<std::string, ConfigurationHandler>	servers;
			std::vector<std::string>					rawFile;
			
			readFile(fileNameCheck(argv[1]), rawFile);
			extractServerBlocks(servers, rawFile);
			
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}
	else
		std::cout << "Please provide a configuration file" << std::endl;
	return 0;
}