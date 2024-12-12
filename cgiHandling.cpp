#include "Types.hpp"
#include <chrono> // for timer building
#include <cstring> // for errno
#include <unistd.h> // fork()
#include <sys/wait.h> // waitpid()
#include <stdlib.h> // getenv()

int	executeCgi(std::string filepath);


int main()
{
	executeCgi("home/cgi-php/index.php");

	return (0);
}


// Returns -1 on failure
int	executeCgi(std::string filepath)
{
	pid_t	cgiPid;
	int		pipeFd[2];
//	std::string	responseBodyStr; // JUST A TEST, use private attributes and include this in the ResponseHandler

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

		std::string interpreterPath;
		std::string interpreterExecName;
		char 		*argArr[3];
		// char *envVarArr[]...?

		// if (content = PHP)
		interpreterPath = "/usr/local/bin/php"; // does this have to be dynamic / searched from the computer...?
		interpreterExecName = "php";
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
		while (waitpidStatus == 0)
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> curTime = std::chrono::high_resolution_clock::now();
			if (curTime - startTime >= std::chrono::seconds(cgiTimeOut))
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


		char 	buffer[1024];
		int		bytesRead;

		bytesRead = read(pipeFd[0], buffer, 1023);
		buffer[bytesRead] = '\0';

		std::cout << "This is what we got:\n\n" << buffer << "\n";

		close(pipeFd[0]);

		/*
			EXIT STATUS HANDLING...?

			if (WIFEXITED(stat_loc) == 1)
				exit_status = WEXITSTATUS(stat_loc);
			else if (WIFSIGNALED(stat_loc) == 1)
			{
				if (WTERMSIG(stat_loc) == 2) --> SIGINT
				{
					exit_status = 130;
				}
				else if (WTERMSIG(stat_loc) == 3)
				{
					exit_status = 131;
				}
			}
		*/

	}
	return (0);

}