#include "Types.hpp"

bool	isSigInt = false;

void	sigIntHandler(int signal)
{
	if (signal == 2)
		isSigInt = true;
}
// Returns -1 for error handling purposes
int	sigIntMessage()
{
	std::cout << GREEN << "\nRecieved SIGINT signal, exiting program\n" << RESET;
	return (-1);
}