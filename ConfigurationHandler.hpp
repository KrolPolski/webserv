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

# define G_CGI_PATH_PHP	"/usr/bin"
# define G_CGI_PATH_PYTHON "/usr/bin"
# define G_METHOD	"GET"

enum dirListStates
{
	FALSE,
	TRUE,
	UNSET
};

struct locationBlock
{
	std::string					m_root;						// root of each location block in server configuration
	int							m_reDirectStatusCode = 0;	// redirect, status code
	std::string					m_reDirectLocation;			// redirect, /new_location
	std::string					m_methods; 					// e.g. GET POST DELETE
	std::string					m_uploadDir;				// specified upload directory
	std::string					m_cgiPathPHP;				// path to cgi interpreter PHP, changes depending on system
	std::string					m_cgiPathPython;			// path to cgi interpreter Python, changes depending on system
	enum dirListStates			m_dirListing; 				// directory listing ON or OFF

	locationBlock() : m_dirListing(UNSET)
	{};
};

class ConfigurationHandler
{
	private:
		enum dirListStates						m_globalDirListing;		// Default false
		std::string								m_globalMethods;		// Default GET
		std::string								m_globalCgiPathPHP;		// Default defined in hpp, changes depending on system
		std::string								m_globalCgiPathPython;	// Default defined in hpp, changes depending on system
		std::map<int, std::string>				m_defaultErrorPages;	// Default for the server if permissions are not in order for custom or not set
		std::vector<std::string>				m_rawBlock;				// the whole server block as cleaned up text
		std::string 							m_host;					// e.g. 127.0.0.1
		std::string 							m_port;					// listen e.g. 8080
		std::string								m_names;				// server names (www.bla.com & bla.com)
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
		enum dirListStates		getInheritedDirListing(std::string) const;
		std::string				getInheritedCgiPathPHP(std::string) const;
		std::string				getInheritedCgiPathPython(std::string) const;
		std::string				getRoot(std::string) const;

		int						getRedirectStatusCode(std::string) const;
		std::string				getRedirectLocation(std::string) const;
		std::string				getUploadDir(std::string) const;

		std::string				getMethods(std::string) const;
		enum dirListStates		getDirListing(std::string) const;
		std::string				getCgiPathPHP(std::string) const;
		std::string				getCgiPathPython(std::string) const;
		std::string				getErrorPages(unsigned int) const;
		std::string				getDefaultErrorPages(unsigned int) const;

		bool					checkLocationBlocksRoot(locationBlock &);
		bool					requiredSettings();

		void					printSettings(); // remove this before we eval -- Patrik

		bool					isRedirectSet(std::string);
		bool					isUploadDirSet(std::string);
		bool					isLocationConfigured(std::string);
};

std::string	fileNameCheck(char *);
void		readFile(const std::string&, std::vector<std::string>&);
void		extractServerBlocks(std::map<std::string, ConfigurationHandler>&, std::vector<std::string>&);

#endif
