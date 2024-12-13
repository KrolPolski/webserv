#include "ResponseHandler.hpp"
#include "CgiHandler.hpp"
#include <cstring> // for errno
#include <unistd.h> // fork()
#include <sys/wait.h> // waitpid()


// Returns -1 on failure
int	CgiHandler::executeCgi(clientInfo *clientPTR, std::string filepath, CgiTypes type)
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

		std::string interpreterPath = "";
		std::string interpreterExecName = "";
		char 		*argArr[3] = {};
		// char *envVarArr[]...? --> Do we need these?

		if (type == PHP)
		{
			interpreterPath = "/usr/bin/php";
			interpreterExecName = "php";
		}
		// else if (content = Python)
		// interpreterPath = ???;
		// interpreterExec = ???;

		argArr[0] = (char *) interpreterExecName.c_str();
		argArr[1] = (char *) filepath.c_str();
		argArr[2] = NULL;

		if (execve(interpreterPath.c_str(), argArr, NULL) == -1)
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
		int	cgiTimeOut = 3; // in seconds

		std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> curTime;

		// Build this differently? Now it slows down other serving if problems occur

		while (waitpidStatus == 0)
		{
			curTime = std::chrono::high_resolution_clock::now();
			if (curTime - startTime >= std::chrono::seconds(cgiTimeOut))
			{
				kill(cgiPid, SIGKILL); // Or sigint...?
				std::cerr << RED << "\nCGI failed due to time out\n" << RESET << "\n";
				// Other error handling?
				return (-1);
			}


			waitpidStatus = waitpid(cgiPid, &statloc, WNOHANG);
	//		std::cout << "PARENT waitpid status: " << waitpidStatus << "\n";

		}



		if (waitpidStatus == -1)
		{
			std::cerr << RED << "\nWaitpid() failed:\n" << RESET << std::strerror(errno) << "\n\n";
			// Other error handling?
			return (-1);
		}

		if (WIFEXITED(statloc) == 1)
		{
			std::cout << "EXIT STATUS: " << WEXITSTATUS(statloc) << "\n";
			if (WEXITSTATUS(statloc) != 0)
				return (-1);
		}
		else if (WIFSIGNALED(statloc) == 1)
		{
			if (WTERMSIG(statloc) == 2)
			{
				std::cout << "EXIT STATUS: SIGINT" << "\n";
			}
			else if (WTERMSIG(statloc) == 3)
			{
				std::cout << "EXIT STATUS: SIGQUIT" << "\n";
			}

			std::cout << "EXIT STATUS: something else" << "\n";

			return (-1);

		}


		char 	buffer[1024]; // what if this is not big enough?
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