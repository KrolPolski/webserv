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

#ifndef CONFIGURATIONHANDLER_HPP
# define CONFIGURATIONHANDLER_HPP

# include <iostream>
# include <vector>
# include <map>
# include <regex>

struct locationBlock
{
	std::string					m_root;
	std::string					m_methods; 		// GET POST DELETE (HEAD)?
	std::string					m_uploadDir;	// upload destination;
	std::string					m_cgiPath;		// path to cgi interpreter
	bool						m_dirListing; 	// directory listing ON or OFF
	//std::string					index; 			// if we are choose to override?

	locationBlock() : m_dirListing(false)
	{};
};

class ConfigurationHandler
{
	private:
		std::vector<std::string>				m_rawBlock;				// the whole server block as cleaned up text
		std::string 							m_host;					// e.g. 127.0.0.1
		std::string 							m_port;					// listen e.g. 8080
		std::string								m_index;				// e.g. index.html
		uint 									m_maxClientBodySize;	// max client body size
		std::string								m_names;				// server names (www.bla.com & bla.com)
		std::map<std::string, locationBlock>	m_routes;				// root of that location as key & struct for each block info
		std::map<int, std::string>				m_redirect;				// HTTP to HTTPS redirection, not much knowledge on this yet
		std::map<int, std::string>				m_errorPages;			// Custom error pages and their location, error code as key

		ConfigurationHandler();

	public:
		ConfigurationHandler(std::vector<std::string>, std::string);
		~ConfigurationHandler() {};

		void		defaultSettings(std::string);

		std::string	getHost() const;
		std::string	getPort() const;
		std::string	getIndex() const;
		uint		getMCBSize() const;
		std::string	getNames() const;
		std::string	getDefaultMethods(std::string) const;
		std::string	getRoot(std::string) const;
		std::string	getMethods(std::string) const;
		std::string	getUploadDir(std::string key) const;
		bool		getDirListing(std::string) const;
		std::string	getCgiPath(std::string key) const;
		std::string	getErrorPages(uint) const;

};

std::string	fileNameCheck(char *);
void		readFile(const std::string&, std::vector<std::string>&);
void		extractServerBlocks(std::map<std::string, ConfigurationHandler>&, std::vector<std::string>&);

#endif