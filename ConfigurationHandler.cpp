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
PRINT SETTINGS
*/

void	ConfigurationHandler::printSettings()
{
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
	std::cout << "\n--------- Default error pages --------------------\n\n";
	for (auto &x : m_defaultErrorPages)
		std::cout << x.first << " : " << x.second << std::endl;
	std::cout << "\n--------- Global DirListing ---------------------------\n\n";
	std::cout << m_globalDirListing << std::endl;
	std::cout << "\n--------- Global Methods ---------------------------\n\n";
	std::cout << m_globalMethods << std::endl;
}

void	ConfigurationHandler::printLocationBlock(locationBlock &block)
{
	if (block.m_root != "")
		std::cout << "\n  " << block.m_root;
	if (block.m_methods != "")
		std::cout << "\n  " << block.m_methods;
	if (block.m_cgiPath != "")
		std::cout << "\n  " << block.m_cgiPath;
	std::cout << "\n  " << block.m_dirListing;
	std::cout << std::endl;
}

/*
DEFAULT SETTINGS
*/

void	ConfigurationHandler::defaultSettings()
{
	// locationBlock loc;
	m_index = "index.html";
	m_maxClientBodySize = 1000000;
	m_globalDirListing = FALSE;
	m_errorPages.emplace(400, "/default-error-pages/400.html");
	m_errorPages.emplace(403, "/default-error-pages/403.html");
	m_errorPages.emplace(404, "/default-error-pages/404.html");
	m_errorPages.emplace(405, "/default-error-pages/405.html");
	m_errorPages.emplace(500, "/default-error-pages/500.html");
	m_defaultErrorPages.emplace(400, "/default-error-pages/400.html");
	m_defaultErrorPages.emplace(403, "/default-error-pages/403.html");
	m_defaultErrorPages.emplace(404, "/default-error-pages/404.html");
	m_defaultErrorPages.emplace(405, "/default-error-pages/405.html");
	m_defaultErrorPages.emplace(500, "/default-error-pages/500.html");
	m_globalMethods = "GET POST";
	// loc.m_root = "home";
	// loc.m_methods = "GET";
	// m_routes.emplace("/", loc);
	printSettings(); //remove before end -- Patrik
}

bool	ConfigurationHandler::checkLocationBlock(locationBlock &block)
{
	printLocationBlock(block);
	if (block.m_root.empty())
		return false;
	// if (block.m_methods == "")
	// 	block.m_methods = getMethods("/");
	// if (m_names == "")
		// return false;
	return true;
}

bool	ConfigurationHandler::requiredSettings()
{
	if (m_port.empty())
	{
		std::cerr << "Error: Port missing!" << std::endl;
		return false;
	}
	if (m_host.empty())
	{
		std::cerr << "Error: IP address missing!" << std::endl;
		return false;
	}
	if (m_names.empty())
	{
		std::cerr << "Error: Server names missing!" << std::endl;
		return false;
	}
	if (m_redirect.empty())
	{
		std::cerr << "Error: Redirect missing!" << std::endl;
		return false;
	}
	// if (!m_routes.contains("/")) /// if no location block is set. globals haave to be given!!!
	
	// 	&& m_routes.find("/cgi/")->second.m_root != "")
	return true;
}

/*
CONSTRUCTOR
*/

ConfigurationHandler::ConfigurationHandler(std::vector<std::string> servBlck) : m_rawBlock(servBlck)
{
	std::cout << "\n\n\nSetting defaults\n";

	defaultSettings();

	std::cout << "\n\n\nBuilding object\n";

	std::regex	listenRegex(R"(^listen\s+(\d+)\s*;\s*$)");
	std::regex	hostRegex(R"(^\s*host\s+([^\s]+)\s*;\s*$)");
	std::regex	serverNameRegex(R"(^\s*server_name\s+([^\s;]+(?:\s+[^\s;]+)*)\s*;\s*$)");
	std::regex	returnRegex(R"(^\s*return\s+(\d+)\s+([^\s]+)\s*;\s*$)");
	std::regex	maxClientBodyRegex(R"(^\s*max_client_body_size\s+(\d+)[Mm]\s*;\s*$)");
	std::regex	errorPageRegex(R"(^\s*error_page\s+(\d+)\s+([^\s]+)\s*;\s*$)");
	std::regex	indexRegex(R"(^\s*index\s+([^\s]+)\s*;\s*$)");
	std::regex	locationRegex(R"(^\s*location\s+([^\s]+)\s*\s*$)");
	std::regex	rootRegex(R"(^\s*root\s+/?([^/][^;]*[^/])?/?\s*;\s*$)");
	std::regex	methodsRegex(R"(^\s*methods\s+([^\s;]+(?:\s+[^\s;]+)*)\s*;\s*$)");
	std::regex	dirListingRegex(R"(^\s*dir_listing\s+(on|off)\s*;\s*$)");
	std::regex	cgiPathRegex(R"(^\s*cgi_path\s+(\/[^/][^;]*[^/])?/?\s*;\s*$)");

	for (std::vector<std::string>::iterator iter = m_rawBlock.begin(); iter != m_rawBlock.end(); iter++)
	{
		std::smatch	match;
		if (std::regex_search(*iter, match, listenRegex) == true)
			m_port = match[1];
		else if (std::regex_search(*iter, match, hostRegex) == true)
			m_host = match[1];
		else if (std::regex_search(*iter, match, serverNameRegex) == true)
			m_names = match[1];
		else if (std::regex_search(*iter, match, returnRegex) == true)
			m_redirect.emplace(std::stoi(match[1]), match[2]);
		else if (std::regex_search(*iter, match, maxClientBodyRegex) == true)
			m_maxClientBodySize = std::stoi(match[1]) * 1000000;
		else if (std::regex_search(*iter, match, errorPageRegex) == true)
		{
			if (m_errorPages.contains(std::stoi(match[1])))
				m_errorPages.erase(std::stoi(match[1]));
			m_errorPages.emplace(std::stoi(match[1]), match[2]);
		}
		else if (std::regex_search(*iter, match, indexRegex) == true)
			m_index = match[1];
		else if (std::regex_search(*iter, match, locationRegex) == true)
		{
			// std::cout << "Processing line: " << *iter << std::endl;
			int openBraces = 0;
			locationBlock loc;
			std::string key = match[1];
			iter++;
			if (iter->find('{') != std::string::npos)
			{
				openBraces++;
				while (openBraces == 1 && iter != m_rawBlock.end())
				{
					// std::cout << "Processing line: " << *iter << std::endl;
					if (std::regex_search(*iter, match, locationRegex) == true)
					{
						openBraces = 0;
						iter--;
						break ;
					}
					std::smatch subMatch;
					if (regex_search(*iter, subMatch, rootRegex) == true)
						loc.m_root = subMatch[1];
					else if (regex_search(*iter, subMatch, methodsRegex) == true)
						loc.m_methods = subMatch[1];
					else if (regex_search(*iter, subMatch, cgiPathRegex) == true)
						loc.m_cgiPath = subMatch[1];
					else if (regex_search(*iter, subMatch, dirListingRegex) == true)
					{
						if (subMatch[1] == "off")
							loc.m_dirListing = FALSE;
						else if (subMatch[1] == "on")
							loc.m_dirListing = TRUE; // if dir listing is not set, we are going to get by default false on it and it will not inhetite from the root "/"
					}
					iter++;
					if (iter->find('}') != std::string::npos)
						openBraces--;
					if (iter->find('{') != std::string::npos)
						openBraces++;
				}
				// std::cout << *iter << std::endl;
				if (openBraces == 0)
				{
					if (checkLocationBlock(loc) == false)
						throw std::runtime_error("Error: Location block not complete"); // this needs more checks in my opinion, depending on the evaluators, what will they test
					if (m_routes.contains(key))
						m_routes.erase(key);
					m_routes.emplace(key, loc);//auto dup = 
					// if (dup.second == false)
					// 	throw std::runtime_error("Error: Duplicate location block found"); // is this extra now when .contains check is right before this?
					loc = locationBlock();
				}
			}
		}
		// std::cout << "Processing line: " << *iter << std::endl;
	}
	printSettings(); //remove before the end of the project -- Patrik // std:::optional
	if (requiredSettings() == false)
		throw std::runtime_error("Error: Location block not complete");
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

uint	ConfigurationHandler::getMCBSize() const
{
	return m_maxClientBodySize;
}

std::string	ConfigurationHandler::getNames() const
{
	return m_names;
}

std::string	ConfigurationHandler::getInheritedMethods(std::string key) const
{
	for (auto &route: m_routes)
	{
		std::string keyFromOurMap = route.first;
		if (keyFromOurMap == "/")
			continue ;
		std::cout << "the key inside getInheritedMethods: " << key << " --- " << keyFromOurMap << std::endl;
		if (key.starts_with(keyFromOurMap))
		{
			std::cout << "match found in " << key << " and " << keyFromOurMap << std::endl;
			if (route.second.m_methods != "")
				return route.second.m_methods;
			else
			{
				std::cout << "Methods not set, inheriting from root" << std::endl;
				return getMethods("/");
			}
		}
	}
	std::cout << "Inheriting from root" << std::endl;
	return getMethods("/"); // if we dont find, we return what the root "/" (home) directory has which we set to defalt if that aswell is missing from the config file
}

enum dirListStates	ConfigurationHandler::getInheritedDirListing(std::string key) const
{
	for (auto &route: m_routes)
	{
		std::string keyFromOurMap = route.first;
		if (keyFromOurMap == "/")
			continue ;
		std::cout << "the key inside getDirListing: " << key << " --- " << keyFromOurMap << std::endl;
		if (key.starts_with(keyFromOurMap))
		{
			std::cout << "match found in " << key << " and " << keyFromOurMap << std::endl;
			if (route.second.m_dirListing != UNSET)
				return route.second.m_dirListing;
			else
			{
				std::cout << "Listing not set, inheriting from root" << std::endl;
				return getDirListing("/");
			}
		}
	}
	std::cout << "Inheriting from root" << std::endl;
	return getDirListing("/");
}

std::string	ConfigurationHandler::getRoot(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
		std::cout << "Error: could not find route for root" << std::endl;
	return map_key->second.m_root;
}

std::string	ConfigurationHandler::getMethods(std::string key) const
{
	// std::cout << "In get methods with key: " << key << "\n";
	// std::cerr << "Available keys:\n";
    // for (auto &route : m_routes)
	// {
    //     std::cerr << " - " << route.first << "\n";
	// }
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
	{
		std::cout << "Error: could not find route for methods" << std::endl;
		return getInheritedMethods(key);
	}
	if (map_key->second.m_methods == "")
		return getInheritedMethods(key);
	// std::cout << "Leaving get methods with " << map_key->second.m_methods << "\n";
	return map_key->second.m_methods;
}

enum dirListStates	ConfigurationHandler::getDirListing(std::string key) const
{
	// std::cout << "In get dirlist with key: " << key << "\n";
	// std::cerr << "Available keys:\n";
    // for (auto &route : m_routes)
	// {
    //     std::cerr << " - " << route.first << "\n";
	// }
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
	{
		std::cout << "Error: could not find route for directory listing" << std::endl;
		return getInheritedDirListing(key);
	}
	if (map_key->second.m_dirListing == UNSET)
		return getInheritedDirListing(key);
	// std::cout << "Leaving get dirlist with " << map_key->second.m_dirListing << "\n";
	return map_key->second.m_dirListing;
}

std::string	ConfigurationHandler::getCgiPath(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
		std::cout << "Error: could not find route cgi interpreter path" << std::endl;
	return map_key->second.m_cgiPath;
}

std::string	ConfigurationHandler::getErrorPages(uint key) const
{
	auto map_key = m_errorPages.find(key);
	if (map_key == m_errorPages.end())
		std::cout << "Error: could not find this " << key << " error pages" << std::endl;
	return map_key->second;
}

std::string	ConfigurationHandler::getDefaultErrorPages(uint key) const
{
	auto map_key = m_errorPages.find(key);
	if (map_key == m_errorPages.end())
		std::cout << "Error: could not find this " << key << " default error pages" << std::endl;
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
