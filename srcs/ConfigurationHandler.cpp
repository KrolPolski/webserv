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
#include "ConnectionHandler.hpp"
#include "Logger.hpp"
#include <fcntl.h>
#include <iostream>
#include <set>
#include <iterator>

/*
PRINT SETTINGS
*/

void	ConfigurationHandler::printSettings()
{
	webservLog.webservLog(INFO, "Printing configuration file server block settings", true);
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
		if (!x.second.m_root.empty())
			std::cout << "\n  " << x.second.m_root;
		if (x.second.m_reDirectStatusCode > 0 && !x.second.m_reDirectLocation.empty())
			std::cout << "\n  " << x.second.m_reDirectStatusCode << " " << x.second.m_reDirectLocation;
		if (!x.second.m_methods.empty())
			std::cout << "\n  " << x.second.m_methods;
		if (!x.second.m_cgiPathPHP.empty())
			std::cout << "\n  " << x.second.m_cgiPathPHP;
		if (!x.second.m_cgiPathPython.empty())
			std::cout << "\n  " << x.second.m_cgiPathPython;
		if (!x.second.m_uploadDir.empty())
			std::cout << "\n  " << x.second.m_uploadDir;
		std::cout << "\n  " << x.second.m_dirListing;
		std::cout << std::endl;
	}
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

/*
DEFAULT/GLOBAL SETTINGS
*/

void	ConfigurationHandler::defaultSettings()
{
	m_index = "index.html";
	m_maxClientBodySize = 1000000;
	m_errorPages.emplace(400, "/default-error-pages/400.html");
	m_errorPages.emplace(403, "/default-error-pages/403.html");
	m_errorPages.emplace(404, "/default-error-pages/404.html");
	m_errorPages.emplace(405, "/default-error-pages/405.html");
	m_errorPages.emplace(408, "/default-error-pages/408.html");
	m_errorPages.emplace(409, "/default-error-pages/409.html");
	m_errorPages.emplace(411, "/default-error-pages/411.html");
	m_errorPages.emplace(413, "/default-error-pages/413.html");
	m_errorPages.emplace(414, "/default-error-pages/414.html");
	m_errorPages.emplace(431, "/default-error-pages/431.html");
	m_errorPages.emplace(500, "/default-error-pages/500.html");
	m_errorPages.emplace(501, "/default-error-pages/501.html");
	m_errorPages.emplace(503, "/default-error-pages/503.html");
	m_errorPages.emplace(505, "/default-error-pages/505.html");
	m_defaultErrorPages.emplace(400, "/default-error-pages/400.html");
	m_defaultErrorPages.emplace(403, "/default-error-pages/403.html");
	m_defaultErrorPages.emplace(404, "/default-error-pages/404.html");
	m_defaultErrorPages.emplace(405, "/default-error-pages/405.html");
	m_defaultErrorPages.emplace(408, "/default-error-pages/408.html");
	m_defaultErrorPages.emplace(409, "/default-error-pages/409.html");
	m_defaultErrorPages.emplace(411, "/default-error-pages/411.html");
	m_defaultErrorPages.emplace(413, "/default-error-pages/413.html");
	m_defaultErrorPages.emplace(414, "/default-error-pages/414.html");
	m_defaultErrorPages.emplace(431, "/default-error-pages/431.html");
	m_defaultErrorPages.emplace(500, "/default-error-pages/500.html");
	m_defaultErrorPages.emplace(501, "/default-error-pages/501.html");
	m_defaultErrorPages.emplace(503, "/default-error-pages/503.html");
	m_defaultErrorPages.emplace(505, "/default-error-pages/505.html");

	m_globalMethods = G_METHOD;
	m_globalDirListing = FALSE;
	m_globalCgiPathPHP = G_CGI_PATH_PHP;
	m_globalCgiPathPython = G_CGI_PATH_PYTHON;
}

bool	ConfigurationHandler::checkLocationBlocksRoot(locationBlock &block)
{
	if (block.m_root.empty())
		return false;
	if (!block.m_root.starts_with("home"))
		return false;
	return true;
}

bool	ConfigurationHandler::requiredSettings()
{
	std::regex	portRegex(R"(^(80|443|8000|8[0-9]{3}|9[0-9]{3})$)");
	std::regex	ipRegex(R"(^([1-9][0-9]{0,2}|1[0-9]{2}|2[0-4][0-9]|25[0-4])\.([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)");
	std::regex	serverNameRegex(R"(^([a-zA-Z0-9-]+)\.([a-zA-Z]{2,})(\s+)(www\.)?([a-zA-Z0-9-]+)\.\2$)");
	std::regex	indexHtmlRegex(R"(^[^.]+\.(html)$)");

	if (m_port.empty())
	{
		webservLog.webservLog(ERROR, "Port missing", false);
		return false;
	}
	if (m_host.empty())
	{
		webservLog.webservLog(ERROR, "IP address missing", false);
		return false;
	}
	if (m_names.empty())
	{
		webservLog.webservLog(ERROR, "Server names missing", false);
		return false;
	}
	if (!m_routes.contains("/"))
	{
		webservLog.webservLog(ERROR, "Home location missing", false);
		return false;
	}
	if (regex_match(m_port, portRegex) == false)
	{
		webservLog.webservLog(ERROR, "Port is not a valid port number", false);
		return false;
	}
	if (regex_match(m_host, ipRegex) == false)
	{
		webservLog.webservLog(ERROR, "IP is not a valid IP address", false);
		return false;
	}
	if (regex_match(m_names, serverNameRegex) == false)
	{
		webservLog.webservLog(ERROR, "Server name is not valid", false);
		return false;
	}
	if (regex_match(m_index, indexHtmlRegex) == false)
	{
		webservLog.webservLog(ERROR, "The index file is not valid", false);
		return false;
	}
	if (m_routes.contains("/"))
	{
		if (m_routes.find("/")->second.m_methods.empty())
		{
			m_routes.find("/")->second.m_methods = m_globalMethods;
		}
		if (m_routes.find("/")->second.m_dirListing == UNSET)
		{
			m_routes.find("/")->second.m_dirListing = m_globalDirListing;
		}
		if (m_routes.find("/")->second.m_cgiPathPHP.empty())
		{
			m_routes.find("/")->second.m_cgiPathPHP= m_globalCgiPathPHP;
		}
		if (m_routes.find("/")->second.m_cgiPathPython.empty())
		{
			m_routes.find("/")->second.m_cgiPathPython = m_globalCgiPathPython;
		}
	}
	return true;
}

/*
CONSTRUCTOR
*/

ConfigurationHandler::ConfigurationHandler(std::vector<std::string> servBlck) : m_rawBlock(servBlck)
{
	webservLog.webservLog(INFO, "Setting default configuration settings", false);

	defaultSettings();

	webservLog.webservLog(INFO, "Building ConfigurationHandler object", false);

	std::regex	listenRegex(R"(^listen\s+(\d+)\s*;\s*$)");
	std::regex	hostRegex(R"(^\s*host\s+([^\s]+)\s*;\s*$)");
	std::regex	serverNameRegex(R"(^\s*server_name\s+([^\s;]+(?:\s+[^\s;]+)*)\s*;\s*$)");
	std::regex	maxClientBodyRegex(R"(^\s*max_client_body_size\s+(\d+)\s*;\s*$)");
	std::regex	errorPageRegex(R"(^\s*error_page\s+(400|403|404|405|408|409|411|413|414|431|500|501|503|505)\s+(/home/[\S]+\.html)\s*;\s*$)");
	std::regex	indexRegex(R"(^\s*index\s+([^\s]+)\s*;\s*$)");
	std::regex	locationRegex(R"(^\s*location\s+([^\s]+)\s*\s*$)");
	std::regex	rootRegex(R"(^\s*root\s+/?([^/][^;]*[^/])?/?\s*;\s*$)");
	std::regex	returnRegex(R"(^\s*return\s+(307)\s+([\S]+)\s*;\s*$)");
	std::regex	methodsRegex(R"(^\s*methods\s+([^\s;]+(?:\s+[^\s;]+)*)\s*;\s*$)");
	std::regex	uploadDirRegex(R"(^\s*upload_dir\s+(home/[\S]+/)\s*;\s*$)");
	std::regex	dirListingRegex(R"(^\s*dir_listing\s+(on|off)\s*;\s*$)");
	std::regex	cgiPathRegexPHP(R"(^\s*cgi_path_php\s+(\/[^/][^;]*[^/])?/?\s*;\s*$)");
	std::regex	cgiPathRegexPython(R"(^\s*cgi_path_python\s+(\/[^/][^;]*[^/])?/?\s*;\s*$)");

	for (std::vector<std::string>::iterator iter = m_rawBlock.begin(); iter != m_rawBlock.end(); iter++)
	{
		std::smatch	match;
		try
		{
			if (regex_search(*iter, match, maxClientBodyRegex) == true)
				m_maxClientBodySize = std::stoul(match[1]);
		}
		catch(const std::exception& e)
		{
			webservLog.webservLog(INFO, "Max client body size conversion failed, using default", true);
		}
		if (regex_search(*iter, match, listenRegex) == true)
			m_port = match[1];
		else if (regex_search(*iter, match, hostRegex) == true)
			m_host = match[1];
		else if (regex_search(*iter, match, serverNameRegex) == true)
			m_names = match[1];
		else if (regex_search(*iter, match, errorPageRegex) == true)
		{
			if (m_errorPages.contains(std::stoi(match[1])))
				m_errorPages.erase(std::stoi(match[1]));
			m_errorPages.emplace(std::stoi(match[1]), match[2]);
		}
		else if (regex_search(*iter, match, indexRegex) == true)
			m_index = match[1];
		else if (regex_search(*iter, match, locationRegex) == true)
		{
			int openBraces = 0;
			locationBlock loc;
			std::string key = match[1];
			iter++;
			if (iter->find('{') != std::string::npos)
			{
				openBraces++;
				while (openBraces == 1 && iter != m_rawBlock.end())
				{
					if (std::regex_search(*iter, match, locationRegex) == true)
					{
						openBraces = 0;
						iter--;
						break ;
					}
					std::smatch subMatch;
					if (regex_search(*iter, subMatch, rootRegex) == true)
						loc.m_root = subMatch[1];
					else if (regex_search(*iter, subMatch, returnRegex) == true)
					{
						loc.m_reDirectStatusCode = std::stoi(subMatch[1]);
						loc.m_reDirectLocation = subMatch[2];
					}
					else if (regex_search(*iter, subMatch, methodsRegex) == true)
						loc.m_methods = subMatch[1];
					else if (regex_search(*iter, subMatch, uploadDirRegex) == true)
						loc.m_uploadDir = subMatch[1];
					else if (regex_search(*iter, subMatch, cgiPathRegexPHP) == true)
						loc.m_cgiPathPHP = subMatch[1];
					else if (regex_search(*iter, subMatch, cgiPathRegexPython) == true)
						loc.m_cgiPathPython = subMatch[1];
					else if (regex_search(*iter, subMatch, dirListingRegex) == true)
					{
						if (subMatch[1] == "off")
							loc.m_dirListing = FALSE;
						else if (subMatch[1] == "on")
							loc.m_dirListing = TRUE;
					}
					iter++;
					if (iter->find('}') != std::string::npos)
						openBraces--;
					if (iter->find('{') != std::string::npos)
						openBraces++;
				}
				if (openBraces == 0)
				{
					if (checkLocationBlocksRoot(loc) == false)
						webservLog.webservLog(WARNING, "Location block not complete, discarding block", true);
					else
					{
						auto duplicate = m_routes.emplace(key, loc);
						if (duplicate.second == false)
							webservLog.webservLog(WARNING, "Duplicate location block found, discarding duplicate", true);
					}
					loc = locationBlock();
				}
			}
		}
	}
	if (requiredSettings() == false)
	{
		webservLog.webservLog(WARNING, "Required settings error", true);
		m_errorStatus = 1;
	}
	else
		printSettings();
}

/*
VALIDATION FOR IF CERTAIN SPECIFICATIONS ARE SET
*/

bool	ConfigurationHandler::isRedirectSet(std::string	key)
{
	auto map_key = m_routes.find(key);
	if (map_key != m_routes.end())
	{
		if (map_key->second.m_reDirectStatusCode > 0 && !map_key->second.m_reDirectLocation.empty())
		{
			webservLog.webservLog(INFO, std::string("Redirect is set in location: " + key + std::string(", redirecting")), true);
			return true;
		}
	}
	webservLog.webservLog(INFO, std::string("Redirect is not set in location: ") + key, false);
	return false;
}

bool	ConfigurationHandler::isUploadDirSet(std::string key)
{
	auto map_key = m_routes.find(key);
	if (map_key != m_routes.end())
	{
		if (!map_key->second.m_uploadDir.empty())
		{
			webservLog.webservLog(INFO, std::string("Upload directory is set in location: " + key), true);
			return true;
		}
	}
	return false;
}

bool	ConfigurationHandler::isLocationConfigured(std::string key)
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
	{
			webservLog.webservLog(ERROR, std::string("Location: " + key + std::string(" is not configured")), true);
			return false;
	}
	return true;
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

int	ConfigurationHandler::getErrorStatus() const
{
	return m_errorStatus;
}

std::string	ConfigurationHandler::getInheritedMethods(std::string key) const
{
	for (auto &route: m_routes)
	{
		std::string keyFromOurMap = route.first;
		if (keyFromOurMap == "/")
			continue ;
		if (key.starts_with(keyFromOurMap))
		{
			if (!route.second.m_methods.empty())
			{
				webservLog.webservLog(INFO, "Found route, returning methods", false);
				return route.second.m_methods;
			}
			else
			{
				webservLog.webservLog(INFO, "Could not find route for methods, inheriting from root", false);
				return getMethods("/");
			}
		}
	}
	webservLog.webservLog(INFO, "Could not find route for methods, inheriting from root", false);
	return getMethods("/"); // if we dont find, we return what the root "/" (home) directory has which we set to defalt if that aswell is missing from the config file
}

enum dirListStates	ConfigurationHandler::getInheritedDirListing(std::string key) const
{
	for (auto &route: m_routes)
	{
		std::string keyFromOurMap = route.first;
		if (keyFromOurMap == "/")
			continue ;
		if (key.starts_with(keyFromOurMap))
		{
			if (route.second.m_dirListing == UNSET)
			{
				webservLog.webservLog(INFO, "Could not find route for directory listing, inheriting from root", false);
				return getDirListing("/");
			}
			webservLog.webservLog(INFO, "Found route, returning directory listing", false);
			return route.second.m_dirListing;
		}
	}
	webservLog.webservLog(INFO, "Could not find route for directory listing, inheriting from root", false);
	return getDirListing("/"); 
}

std::string	ConfigurationHandler::getInheritedCgiPathPHP(std::string key) const
{
	for (auto &route: m_routes)
	{
		std::string keyFromOurMap = route.first;
		if (keyFromOurMap == "/")
			continue ;
		if (key.starts_with(keyFromOurMap))
		{
			if (!route.second.m_cgiPathPHP.empty())
			{
				return route.second.m_cgiPathPHP;
			}
			else
			{
				webservLog.webservLog(INFO, "Could not find route for cgi path, inheriting from root", false);
				return getCgiPathPHP("/");
			}
		}
	}
	webservLog.webservLog(INFO, "Could not find route for cgi path, inheriting from root", false);
	return getCgiPathPHP("/");
}

std::string	ConfigurationHandler::getInheritedCgiPathPython(std::string key) const
{
	for (auto &route: m_routes)
	{
		std::string keyFromOurMap = route.first;
		if (keyFromOurMap == "/")
			continue ;
		if (key.starts_with(keyFromOurMap))
		{
			if (!route.second.m_cgiPathPython.empty())
			{
				return route.second.m_cgiPathPython;
			}
			else
			{
				webservLog.webservLog(INFO, "Could not find route for cgi path, inheriting from root", false);
				return getCgiPathPython("/");
			}
		}
	}
	webservLog.webservLog(INFO, "Could not find route for cgi path, inheriting from root", false);
	return getCgiPathPython("/");
}

std::string	ConfigurationHandler::getRoot(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
	{
		webservLog.webservLog(ERROR, "Could not find route for root", false);
		return "";
	}
	return map_key->second.m_root;
}


int	ConfigurationHandler::getRedirectStatusCode(std::string key) const
{
	auto map_key = m_routes.find(key);
	return map_key->second.m_reDirectStatusCode;
}

std::string	ConfigurationHandler::getRedirectLocation(std::string key) const
{
	auto map_key = m_routes.find(key);
	return map_key->second.m_reDirectLocation;
}

std::string	ConfigurationHandler::getUploadDir(std::string key) const
{
	auto map_key = m_routes.find(key);
	return map_key->second.m_uploadDir;
}

std::string	ConfigurationHandler::getMethods(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
	{
		webservLog.webservLog(INFO, "Could not find route for methods, inheriting", false);
		return getInheritedMethods(key);
	}
	if (map_key->second.m_methods.empty())
		return getInheritedMethods(key);
	webservLog.webservLog(INFO, "Found route for methods in " + key, false);
	return map_key->second.m_methods;
}

enum dirListStates	ConfigurationHandler::getDirListing(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
	{
		webservLog.webservLog(INFO, "Could not find route for directory listing, inheriting", false);
		return getInheritedDirListing(key);
	}
	if (map_key->second.m_dirListing == UNSET)
	{
		webservLog.webservLog(INFO, "Could not find route for directory listing, inheriting", false);
		return getInheritedDirListing(key);
	}
	webservLog.webservLog(INFO, "Found route for directory listing in " + key, false);
	return map_key->second.m_dirListing;
}

std::string	ConfigurationHandler::getCgiPathPHP(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
	{
		webservLog.webservLog(ERROR, "Could not find route cgi interpreter path", false);
		return getInheritedCgiPathPHP(key);
	}
	if (map_key->second.m_cgiPathPHP.empty())
  	{
		webservLog.webservLog(ERROR, "Could not find route cgi interpreter path", false);
    return getInheritedCgiPathPHP(key);
  	}
	webservLog.webservLog(INFO, "Found route for cgi php path in " + key, false);
	return map_key->second.m_cgiPathPHP;
}

std::string	ConfigurationHandler::getCgiPathPython(std::string key) const
{
	auto map_key = m_routes.find(key);
	if (map_key == m_routes.end())
	{
		webservLog.webservLog(ERROR, "Could not find route cgi interpreter path", false);
		return getInheritedCgiPathPython(key);
	}
	if (map_key->second.m_cgiPathPython.empty())
  	{
		webservLog.webservLog(ERROR, "Could not find route cgi interpreter path", false);
    return getInheritedCgiPathPython(key);
  	}
	webservLog.webservLog(INFO, "Found route for cgi python path in " + key, false);
	return map_key->second.m_cgiPathPython;
}

std::string	ConfigurationHandler::getErrorPages(uint key) const
{
	auto map_key = m_errorPages.find(key);
	if (map_key == m_errorPages.end())
	{
		webservLog.webservLog(ERROR, "Could not find custom error page, using default instead", false);
		return "";
	}
	return map_key->second;
}

std::string	ConfigurationHandler::getDefaultErrorPages(uint key) const
{
	auto map_key = m_defaultErrorPages.find(key);
	if (map_key == m_defaultErrorPages.end())
	{
		webservLog.webservLog(ERROR, "Could not find default error page, sending code 500 instead", false);
		return "";
	}
	return map_key->second;
}

/*
CHECK THE FILE NAME
*/

std::string	fileNameCheck(char *argv)
{
	std::string	file = argv;

	if (regex_match(file, std::regex(".*\\.conf$")) == false)
		throw std::runtime_error("Configuration file could not be found");
	return file;
}

/*
READ THE FILE
*/

void	readFile(const std::string &fileName, std::vector<std::string> &rawFile)
{
	webservLog.webservLog(INFO, "Reading configuration file", false);
	std::string		line;
	std::ifstream	file(fileName);
	int				curlyBrace = 0;

	if (!file.is_open())
		throw std::runtime_error("Failed to open the configuration file");
	while (getline(file, line))
	{
		if (file.fail())
			throw std::runtime_error("Reading the file failed");
		if (line.find('{') != line.npos)
			curlyBrace++;
		if (line.find('}') != line.npos)
			curlyBrace--;
		line = regex_replace(line, std::regex("^\\s+|\\s+$"), "");
		size_t comment = line.find('#');
		if (comment != std::string::npos)
			line = line.substr(0, comment);
		line = regex_replace(line, std::regex("^\\s+|\\s+$"), "");
		if (!line.empty())
			rawFile.push_back(line);
	}
	if (file.fail() && !file.eof())
		throw std::runtime_error("Reading the file failed");
	file.close();
	if (curlyBrace != 0)
		throw std::runtime_error("Open curly braces in the configuration file");
	if (rawFile.empty())
		throw std::runtime_error("The given configuration file doesn't have any readable input");
}

/*
EXTRACTING EACH SERVER BLOCK
*/

void	extractServerBlocks(std::multimap<std::string, ConfigurationHandler> &servers, std::vector<std::string> &rawFile)
{
	webservLog.webservLog(INFO, "Extracting server blocks from Configuration file", false);

	std::string					port;
	int							openBraces = 0;
	bool						duplicateIP = false;
	std::vector<std::string>	temp;
	std::set<std::string>		ipAddresses;

	for (std::vector<std::string>::iterator iter = rawFile.begin(); iter != rawFile.end(); iter++)
	{
		std::smatch	match;
		if (regex_search(*iter, match, std::regex("^server$")) == true)
		{
			temp.push_back(*iter);
			iter++;
		}
		if (regex_search(*iter, match, std::regex(R"(^listen\s+(\d+)\s*;\s*$)")) == true)
			port = match[1];
		if (regex_search(*iter, match, std::regex(R"(^\s*host\s+([^\s]+)\s*;\s*$)")) == true)
		{
			auto dupIpCheck = ipAddresses.insert(match[1]);
			if (dupIpCheck.second == false)
			{
				webservLog.webservLog(WARNING, "Duplicate IP address found, discarding duplicate server block", true);
				duplicateIP = true;
			}
		}
		if (iter->find('{') != std::string::npos)
			openBraces++;
		if (iter->find('}') != std::string::npos)
			openBraces--;
		if (openBraces == 0)
		{
			temp.push_back(*iter);
			if (duplicateIP == false)
				servers.emplace(port, ConfigurationHandler(temp));
			auto last = std::prev(servers.end());
			if (last->second.getErrorStatus() == 1)
				servers.erase(last);
			temp.clear();
			port.clear();
			duplicateIP = false;
		}
		if (!temp.empty())
			temp.push_back(*iter);
	}
}
