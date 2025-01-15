#include <iostream>

bool	isSigInt = false;

void	sigIntHandler(int signal)
{
	if (signal == 2)
		isSigInt = true;
}
