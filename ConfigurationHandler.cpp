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
	std::cout << "\n\n\nBuilding object\n";

	std::regex	listenRegex(R"(^listen\s+(\d+)\s*;\s*$)");
	std::regex	hostRegex(R"(^\s*host\s+([^\s]+)\s*;\s*$)");
	std::regex	serverNameRegex(R"(^\s*server_name\s+([^\s;]+(?:\s+[^\s;]+)*)\s*;\s*$)");
	std::regex	returnRegex(R"(^\s*return\s+(\d+)\s+([^\s]+)\s*;\s*$)");
	std::regex	maxClientBodyRegex(R"(^\s*max_client_body_size\s+(\d+)[Mm]\s*;\s*$)");
	std::regex	errorPageRegex(R"(^\s*error_page\s+(\d+)\s+([^\s]+)\s*;\s*$)");
	std::regex	indexRegex(R"(^\s*index\s+([^\s]+)\s*;\s*$)");
	std::regex	locationRegex(R"(^\s*location\s+([^\s]+)\s*\s*$)");

	for (std::vector<std::string>::iterator iter = m_rawBlock.begin(); iter != m_rawBlock.end(); iter++)
	{
		std::smatch	match;
		if (std::regex_search(*iter, match, listenRegex) == true)
			m_port = match[1];
		if (std::regex_search(*iter, match, hostRegex) == true)
			m_host = match[1];
		if (std::regex_search(*iter, match, serverNameRegex) == true)
			m_names = match[1];
		if (std::regex_search(*iter, match, returnRegex) == true)
			m_redirect.emplace(std::stoi(match[1]), match[2]);
		if (std::regex_search(*iter, match, maxClientBodyRegex) == true)
			m_maxClientBodySize = std::stoi(match[1]);
		if (std::regex_search(*iter, match, errorPageRegex) == true)
			m_errorPages.emplace(std::stoi(match[1]), match[2]);
		if (std::regex_search(*iter, match, indexRegex) == true)
			m_index = match[1];
		if (std::regex_search(*iter, match, locationRegex) == true)
		{
			std::regex	rootRegex(R"(^\s*root\s+/?([^/][^;]*[^/])?/?\s*;\s*$)");
			std::regex	methodsRegex(R"(^\s*methods\s+([^\s;]+(?:\s+[^\s;]+)*)\s*;\s*$)");
			std::regex	dirListingRegex(R"(^\s*dir_listing\s+(on|off)\s*;\s*$)");
			int openBraces = 0;
			locationBlock loc;
			std::string key = match[1];
			iter++;
			if (iter->find('{') != std::string::npos)
			{
				openBraces++;
				while (openBraces == 1 && iter != m_rawBlock.end())
				{
					std::smatch subMatch;
					iter++;
					if (iter->find('}') != std::string::npos)
						openBraces--;
					if (regex_search(*iter, subMatch, rootRegex) == true)
						loc.m_root = subMatch[1];
					if (regex_search(*iter, subMatch, methodsRegex) == true)
						loc.m_methods = subMatch[1];
					if (regex_search(*iter, subMatch, dirListingRegex) == true)
					{
						std::string temp = iter->substr(12, iter->size() - 13);
						if (temp == "off")
							loc.m_dirListing = false;
						else if (temp == "on")
							loc.m_dirListing = true;
					}
					if (openBraces == 0)
					{
						auto dup = m_routes.emplace(key, loc);
						if (dup.second == false)
							throw std::runtime_error("Error: Duplicate location block found");
						loc = locationBlock();
					}
				}
			}
		}
	}
	std::cout << std::boolalpha;
	std::cout << "\n--------- Port -----------------------------------\n\n";
	std::cout << m_port << std::endl;
	std::cout << "\n--------- Host -----------------------------------\n\n";
	std::cout << m_host << std::endl;
	std::cout << "\n--------- Index ----------------------------------\n\n";
	std::cout << m_index << std::endl;
	std::cout << "\n--------- Max client body size -------------------\n\n";
	std::cout << m_maxClientBodySize << std::endl;
	std::cout << "\n--------- Server names ---------------------------\n\n";
	std::cout << m_names << std::endl;
	std::cout << "\n--------- Routes ---------------------------------\n";
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

std::string	ConfigurationHandler::getHost() const
{
	return m_host;
}

std::string	ConfigurationHandler::getPort() const
{
	return m_port;
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

std::string	ConfigurationHandler::getRoot(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
		std::cout << "Error: could not find route" << std::endl;
	return map_key->second.m_root;
}

std::string	ConfigurationHandler::getMethods(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
		std::cout << "Error: could not find route" << std::endl;
	return map_key->second.m_methods;
}

bool	ConfigurationHandler::getDirListing(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
		std::cout << "Error: could not find route" << std::endl;
	return map_key->second.m_dirListing;
}

std::string	ConfigurationHandler::getErrorPages(uint key) const
{
	auto map_key = m_errorPages.find(key);
	if (map_key == m_errorPages.end())
		std::cout << "Error: could not find this error page directory" << std::endl;
	return map_key->second;
}

/*
CHECK THE FILE NAME
*/

// this is not final!! ----- Patrik

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
		throw std::runtime_error("Error: Failed to open the configuration file");
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
		std::cerr << "Error: Reading the file: " << e.what() << std::endl;
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
			std::smatch	match;
			if (std::regex_search(*iter, match, std::regex("^server$")) == true)
			{
				temp.push_back(*iter);
				iter++;
			}
			if (std::regex_search(*iter, match, std::regex(R"(^listen\s+(\d+)\s*;\s*$)")) == true)
				port = match[1];
			if (iter->find('{') != std::string::npos)
				openBraces++;
			if (iter->find('}') != std::string::npos)
				openBraces--;
			if (openBraces == 0)
			{
				temp.push_back(*iter);
				auto dup = servers.emplace(port, ConfigurationHandler(temp));
				if (dup.second == false)
					throw std::runtime_error("Error: Duplicate port found");
				std::cout << servers.size() << " -------------------------- Checking size\n";
				if (servers.size() > 5)
					throw std::runtime_error("Error: Configuration file is too big");
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