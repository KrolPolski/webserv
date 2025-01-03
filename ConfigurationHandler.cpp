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
DEFAULT SETTINGS
*/

void	ConfigurationHandler::defaultSettings(std::string port)
{
	locationBlock loc;
	m_port = port;
	m_host = "127.0.0.1";
	m_index = "index.html";
	loc.m_root = "home";
	loc.m_methods = "GET POST DELETE";
	m_routes.emplace("/", loc);

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
	{
		std::cout 
		<< "\n" << x.first;
		if (x.second.m_root != "")
			std::cout << "\n  " << x.second.m_root;
		if (x.second.m_methods != "")
			std::cout << "\n  " << x.second.m_methods;
		if (x.second.m_uploadDir != "")
			std::cout << "\n  " << x.second.m_uploadDir;
		if (x.second.m_cgiPath != "")
			std::cout << "\n  " << x.second.m_cgiPath;
		std::cout << "\n  " << x.second.m_dirListing;
		std::cout << std::endl;
	}
	std::cout << "\n--------- Redirects ------------------------------\n\n";
	for (auto &x : m_redirect)
		std::cout << x.first << " : " << x.second << std::endl;
	std::cout << "\n--------- Error pages ----------------------------\n\n";
	for (auto &x : m_errorPages)
		std::cout << x.first << " : " << x.second << std::endl;
}
bool	ConfigurationHandler::checkLocationBlock(locationBlock block)
{
	if (block.m_root == "")
		return false;
	if (block.m_methods == "")
		return false;
	return true;
}

/*
CONSTRUCTOR
*/

ConfigurationHandler::ConfigurationHandler(std::vector<std::string> servBlck, std::string port) : m_rawBlock(servBlck)
{
	std::cout << "\n\n\nSetting defaults\n";

	defaultSettings(port);

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
			std::regex	uploadDirRegex(R"(^\s*upload_dir\s+/?([^/][^;]*[^/])?/?\s*;\s*$)");
			std::regex	cgiPathRegex(R"(^\s*cgi_path\s+(\/[^/][^;]*[^/])?/?\s*;\s*$)");
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
					if (regex_search(*iter, subMatch, uploadDirRegex) == true)
						loc.m_uploadDir = subMatch[1];
					if (regex_search(*iter, subMatch, cgiPathRegex) == true)
						loc.m_cgiPath = subMatch[1];
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
						if (checkLocationBlock(loc) == false)
							throw std::runtime_error("Error: Location block not complete"); // this needs more checks in my opinion, depending on the evaluators, what will they test
						if (m_routes.count(key) == 1)
							m_routes.erase(key); // Here i need to check if we had something in the block and then emplace. ---- Patrik
						auto dup = m_routes.emplace(key, loc); // this might be an issue if we erase -- Patrik
						if (dup.second == false)
							throw std::runtime_error("Error: Duplicate location block found"); // is this extra now when .count check is right before this?
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
	{
		std::cout 
		<< "\n" << x.first;
		if (x.second.m_root != "")
			std::cout << "\n  " << x.second.m_root;
		if (x.second.m_methods != "")
			std::cout << "\n  " << x.second.m_methods;
		if (x.second.m_uploadDir != "")
			std::cout << "\n  " << x.second.m_uploadDir;
		if (x.second.m_cgiPath != "")
			std::cout << "\n  " << x.second.m_cgiPath;
		std::cout << "\n  " << x.second.m_dirListing;
		std::cout << std::endl;
	}
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

std::string	ConfigurationHandler::getDefaultMethods(std::string key) const
{
	for (auto &route: m_routes)
	{
		std::string keyFromOurMap = route.first;
		if (key.rfind(keyFromOurMap, 0) == 0)
			return route.second.m_methods;
	}
	return "GET POST DELETE";
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
	{
		std::cout << "Error: could not find route, checking defaults" << std::endl;
		return getDefaultMethods(key);
	}
	return map_key->second.m_methods;
}

std::string	ConfigurationHandler::getUploadDir(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
		std::cout << "Error: could not find route" << std::endl;
	return map_key->second.m_uploadDir;
}

bool	ConfigurationHandler::getDirListing(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
		std::cout << "Error: could not find route" << std::endl;
	return map_key->second.m_dirListing;
}

std::string	ConfigurationHandler::getCgiPath(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
		std::cout << "Error: could not find route" << std::endl;
	return map_key->second.m_cgiPath;
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

	if (std::regex_match(file, std::regex(".*\\.conf$")) == false)
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
	int				curlyBrace = 0;

	if (!file.is_open())
		throw std::runtime_error("Error: Failed to open the configuration file");
	try
	{
		while (getline(file, line))
		{
			if (line.find('{') != line.npos)
				curlyBrace++;
			if (line.find('}') != line.npos)
				curlyBrace--;
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
	if (curlyBrace != 0)
		throw std::runtime_error("Open curly braces in the configuration file");
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
				auto dup = servers.emplace(port, ConfigurationHandler(temp, port));
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