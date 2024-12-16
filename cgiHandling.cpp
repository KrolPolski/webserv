#include "ResponseHandler.hpp"
#include "CgiHandler.hpp"
#include <cstring> // for errno
#include <unistd.h> // fork()
#include <sys/wait.h> // waitpid()


// Returns -1 on failure
int	CgiHandler::executeCgi(clientInfo *clientPTR)
{
	pid_t	cgiPid;
	int		pipeFd[2];

	if (pipe(pipeFd) == -1)
	{
		std::cerr << RED << "\nPipe() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		// Other error handling?
		return (-1);
	}

	cgiPid = fork();
	if (cgiPid == -1)
	{
		std::cerr << RED << "\nFork() failed:\n" << RESET << std::strerror(errno) << "\n\n";
		close(pipeFd[0]);
		close(pipeFd[1]);

		// Other error handling?
		return (-1);
	}

	if (cgiPid == 0)
	{
		// chdir() to right folder...?

		close (pipeFd[0]); // do we need to check for errors with close()...?


		if (dup2(pipeFd[1], STDOUT_FILENO) == -1)
		{
			std::cerr << RED << "\nDup2() failed:\n" << RESET << std::strerror(errno) << "\n\n";
			// Other error handling?
			return (-1);	
		}


		if (execve(m_pathToInterpreter.c_str(), m_argsForExecve, m_envArrExecve) == -1)
		{
			close(pipeFd[1]);
			std::cerr << RED << "\nExecve() failed:\n" << RESET << std::strerror(errno) << "\n\n";
			// Other error handling?
			return (1); // or some other exit code...?
		}
	}
	else
	{

		close(pipeFd[1]);
		int statloc;
		int	waitpidStatus = 0;

		/*
			PANU:
			Evantually do this timeout check in the poll loop!
		*/
		std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> curTime;

		while (waitpidStatus == 0)
		{
			curTime = std::chrono::high_resolution_clock::now();
			if (curTime - startTime >= std::chrono::seconds(m_cgiTimeOut))
			{
				kill(cgiPid, SIGKILL); // Or sigint...?
				std::cerr << RED << "\nCGI failed due to time out\n" << RESET << "\n";
				// Other error handling?
				return (-1);
			}

			waitpidStatus = waitpid(cgiPid, &statloc, WNOHANG);
		}



		if (waitpidStatus == -1)
		{
			std::cerr << RED << "\nWaitpid() failed:\n" << RESET << std::strerror(errno) << "\n\n";
			// Other error handling?
			return (-1);
		}

		if (WIFEXITED(statloc) == 1)
		{
			if (WEXITSTATUS(statloc) != 0)
				return (-1);
		}
		else if (WIFSIGNALED(statloc) == 1)
		{
			if (WTERMSIG(statloc) == 2)
				std::cout << "EXIT STATUS: SIGINT" << "\n"; // what should we do...?
			else if (WTERMSIG(statloc) == 3)
				std::cout << "EXIT STATUS: SIGQUIT" << "\n"; // what should we do...?
			return (-1);
		}


		/*
			PANU:

			This needs a rebuild. 
			It needs to happen like the response building: read one chunk per round, and check if all has been read.
			Also: Do I need to do poll() check before entering this...?
		*/
		char 	buffer[1024];
		int		bytesRead;

		bytesRead = read(pipeFd[0], buffer, 1023); // This needs to go thorugh poll()...?

		if (bytesRead == -1)
		{
			std::cerr << RED << "\nRead() failed:\n" << RESET << std::strerror(errno) << "\n\n";
			// Other error handling?
			return (-1);
		}

		buffer[bytesRead] = '\0';

		clientPTR->responseBody = buffer;

		close(pipeFd[0]);
	}
	return (0);

}