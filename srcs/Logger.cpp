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

#include "Logger.hpp"
#include "Types.hpp"

Logger::Logger(const std::string &log_file)
{
	logFile.open(log_file, std::ios::out);
	if (!logFile.is_open())
	{
		std::cerr << "Error: opening log file failed" << std::endl;
		exit(1);
	}
}

Logger::~Logger()
{
	logFile.close();
}

void	Logger::webservLog(LogStatus status, const std::string& message, bool toTerminal) // variadic template?????
{
	time_t	now = time(0);
	tm*		timeinfo = localtime(&now);
	char	timestamp[20];

	strftime(timestamp, sizeof(timestamp), "%Y-%d-%m %H:%M:%S", timeinfo);
	std::ostringstream	logEntry;

	switch (status)
	{
		case INFO:
		{
			logEntry << LIGHT_GREEN << "[" << timestamp << "] " << ORANGE << "INFO" << ": " << RESET << message << std::endl;
			break;
		}
		case ERROR:
		{
			logEntry << LIGHT_GREEN << "[" << timestamp << "] " << RED << "ERROR" << ": " << RESET << message << std::endl;
			break;
		}
		case DEBUG:
		{
			logEntry << LIGHT_GREEN << "[" << timestamp << "] " << CYAN << "DEBUG" << ": " << RESET << message << std::endl;
			break;
		}
		case WARNING:
		{
			logEntry << LIGHT_GREEN << "[" << timestamp << "] " << PURPLE << "WARNING" << ": " << RESET << message << std::endl;
			break;
		}
		default:
		{
			logEntry << LIGHT_GREEN << "[" << timestamp << "] " << BLUE << "UNKNOWN" << ": " << RESET << message << std::endl;
			break;
		}
	}
	if (toTerminal)
	{
		std::cout << logEntry.str();
	}
	logFile << logEntry.str();
	logFile.flush();
}

void	Logger::closeLogFileStream()
{
	logFile.close();
}
