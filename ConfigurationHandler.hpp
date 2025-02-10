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

# include <vector>
# include <map>
# include <regex>

# define G_CGI_PATH	"/usr/bin"
# define G_METHOD	"GET"

enum dirListStates
{
	FALSE,
	TRUE,
	UNSET
};

struct locationBlock
{
	std::string					m_root;
	std::string					m_methods; 		// GET POST DELETE
	std::string					m_cgiPath;		// path to cgi interpreter
	enum dirListStates			m_dirListing; 	// directory listing ON or OFF

	locationBlock() : m_dirListing(UNSET)
	{};
};

class ConfigurationHandler
{
	private:
		enum dirListStates						m_globalDirListing;
		std::string								m_globalMethods;
		std::string								m_globalCgiPath;
		std::map<int, std::string>				m_defaultErrorPages;	// Default for the server if permissions are not in order for custom

		std::vector<std::string>				m_rawBlock;				// the whole server block as cleaned up text
		std::string 							m_host;					// e.g. 127.0.0.1
		std::string 							m_port;					// listen e.g. 8080
		std::string								m_names;				// server names (www.bla.com & bla.com)
		std::map<int, std::string>				m_redirect;				// HTTP to HTTPS redirection, not much knowledge on this yet
		std::string								m_index;				// e.g. index.html
		unsigned int 							m_maxClientBodySize;	// max client body size
		std::map<std::string, locationBlock>	m_routes;				// root of that location as key & struct for each block info
		std::map<int, std::string>				m_errorPages;			// Custom error pages and their location, error code as key

		ConfigurationHandler();

	public:
		ConfigurationHandler(std::vector<std::string>);
		~ConfigurationHandler() {};

		void		defaultSettings();

		std::string				getHost() const;
		std::string				getPort() const;
		std::string				getIndex() const;
		unsigned int			getMCBSize() const;
		std::string				getNames() const;
		std::string				getInheritedMethods(std::string) const;
		enum dirListStates		getInheritedDirListing(std::string key) const;
		std::string				getInheritedCgiPath(std::string) const;
		std::string				getRoot(std::string) const;
		std::string				getMethods(std::string) const;
		enum dirListStates		getDirListing(std::string) const;
		std::string				getCgiPath(std::string key) const;
		std::string				getErrorPages(unsigned int) const;
		std::string				getDefaultErrorPages(unsigned int) const;

		bool					checkLocationBlocksRoot(locationBlock &);
		bool					requiredSettings();

		void					printSettings(); // remove this before we eval -- Patrik
};

std::string	fileNameCheck(char *);
void		readFile(const std::string&, std::vector<std::string>&);
void		extractServerBlocks(std::map<std::string, ConfigurationHandler>&, std::vector<std::string>&);

#endif
