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
#include <fcntl.h>
#include <fstream>

/*
CONSTRUCTOR
*/

ConfigurationHandler::ConfigurationHandler(std::vector<std::string> servBlck) : m_rawBlock(servBlck)
{
	std::cout << "\n\n\n\nBuilding object\n";
	for (std::vector<std::string>::iterator iter = m_rawBlock.begin(); iter != m_rawBlock.end(); iter++)
	{
		if (iter->find("listen") != std::string::npos)
			m_port = iter->substr(7, iter->size() - 8);
		if (iter->find("host") != std::string::npos)
			m_host = iter->substr(5, iter->size() - 6);
		if (iter->find("server_name") != std::string::npos)
			m_names = iter->substr(12, iter->size() - 13);
		if (iter->find("return") != std::string::npos)
			m_redirect.emplace(0, iter->substr(7, iter->size() - 8));
		if (iter->find("max_client_body_size") != std::string::npos)
			m_maxClientBodySize = std::stoi(iter->substr(21, iter->size() - 23));
		if (iter->find("error_page") != std::string::npos)
			m_errorPages.emplace(std::stoi(iter->substr(11, 3)), iter->substr(15, iter->size() - 16));
		if (iter->find("index") != std::string::npos)
			m_index = iter->substr(6, iter->size() - 7);
		if (iter->find("location") != std::string::npos)
		{
			int openBraces = 0;
			locationBlock loc;
			std::string key = iter->substr(9, iter->size() - 9);
			iter++;
			if (iter->find('{') != std::string::npos)
			{
				openBraces++;
				while (openBraces == 1)
				{
					iter++;
					if (iter->find('}') != std::string::npos)
						openBraces--;
					if (iter->find("root") != std::string::npos)
						loc.m_root = iter->substr(5, iter->size() - 6);
					if (iter->find("methods") != std::string::npos)
						loc.m_methods = iter->substr(8, iter->size() - 9);
					if (iter->find("dir_listing") != std::string::npos)
					{
						std::string temp = iter->substr(12, iter->size() - 13);
						if (temp == "off")
							loc.m_dirListing = false;
						else if (temp == "on")
							loc.m_dirListing = true;
					}
					if (openBraces == 0)
					{
						m_routes.emplace(key, loc);
						loc = locationBlock();
					}
				}
			}
		}
	}
	std::cout << std::boolalpha;
	std::cout << "\n--------- Host -----------------------------------\n\n";
	std::cout << m_host << std::endl;
	std::cout << "\n--------- Port -----------------------------------\n\n";
	std::cout << m_port << std::endl;
	std::cout << "\n--------- Index ----------------------------------\n\n";
	std::cout << m_index << std::endl;
	std::cout << "\n--------- Max client body size -------------------\n\n";
	std::cout << m_maxClientBodySize << std::endl;
	std::cout << "\n--------- Server names ---------------------------\n\n";
	std::cout << m_names << std::endl;
	std::cout << "\n--------- Routes ---------------------------------\n\n";
	for (auto &x : m_routes)
		std::cout << "\n" << x.first << "\n  " << x.second.m_root << "\n  " << x.second.m_methods << "\n  " << x.second.m_dirListing << std::endl;
	std::cout << "\n--------- Redirects ------------------------------\n\n";
	for (auto &x : m_redirect)
		std::cout << x.first << " : " << x.second << std::endl;
	std::cout << "\n--------- Error pages ----------------------------\n\n";
	for (auto &x : m_errorPages)
		std::cout << x.first << " : " << x.second << std::endl;
}


/*
GETTERS
*/

std::string	ConfigurationHandler::getPort() const
{
	return m_port;
}

std::string	ConfigurationHandler::getHost() const
{
	return m_host;
}

std::string	ConfigurationHandler::getIndex() const
{
	return m_index;
}

uint		ConfigurationHandler::getMCBSize() const
{
	return m_maxClientBodySize;
}

std::string	ConfigurationHandler::getNames() const
{
	return m_names;
}

/*
CHECK THE FILE NAME
*/

std::string	fileNameCheck(char *argv)
{
	std::cout << "Checking\n\n";
	std::string	file = argv;

	if (std::regex_match(file, std::regex("^web.conf$")) == false)
		throw std::runtime_error("Wrong configuration file");
	return file;
}

/*
READ THE FILE
*/

void	readFile(const std::string &fileName, std::vector<std::string> &rawFile)
{
	std::cout << "Reading\n\n";
	std::string		line;
	std::ifstream	file(fileName);

	if (!file.is_open())
		throw std::runtime_error("Failed to open the configuration file");
	try
	{
		while (getline(file, line))
		{
			line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");
			size_t comment = line.find('#');
			if (comment != std::string::npos)
				line = line.substr(0, comment);
			line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");
			if (!line.empty())
				rawFile.push_back(line);
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error while reading the file: " << e.what() << std::endl;
		file.close();
		throw;
	}
	file.close();
}

/*
EXTRACTING EACH SERVER BLOCK
*/

void	extractServerBlocks(std::map<std::string, ConfigurationHandler> &servers, std::vector<std::string> &rawFile)
{
	std::cout << "Extracting\n\n";
	try
	{
		std::string	port;
		int	openBraces = 0;
		std::vector<std::string>	temp;
		for (std::vector<std::string>::iterator iter = rawFile.begin(); iter != rawFile.end(); iter++)
		{
			if (std::regex_match(*iter, std::regex("^server$")))
			{
				temp.push_back(*iter);
				iter++;
			}
			if (iter->find("listen") != std::string::npos)
				port = iter->substr(7, iter->size() - 8);
			if (iter->find('{') != std::string::npos)
				openBraces++;
			if (iter->find('}') != std::string::npos)
				openBraces--;
			if (openBraces == 0)
			{
				temp.push_back(*iter);
				servers.emplace(port, ConfigurationHandler(temp));
				temp.clear();
				port.clear();
			}
			if (!temp.empty())
				temp.push_back(*iter);
		}
		
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}