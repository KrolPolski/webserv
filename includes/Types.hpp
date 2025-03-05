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

#pragma once

#include <signal.h>

#define RED 	"\033[31m"
#define GREEN	"\033[32m"
#define BLUE	"\033[34m"
#define PURPLE	"\033[35m"
#define CYAN   "\033[36m"
#define YELLOW  "\033[33m"
#define LIGHT_GREEN "\033[92m"
#define ORANGE "\033[38;5;214m"
#define RESET	"\033[0m"

enum requestTypes
{
	GET,
	POST,
	DELETE,
	INVALID
};

enum CgiTypes
{
	NONE,
	PHP,
	PYTHON
};

enum clientStatus
{
	RECIEVE_REQUEST,
	PARSE_REQUEST,
	SAVE_FILE,
	BUILD_ERRORPAGE,
	BUILD_RESPONSE,
	EXECUTE_CGI,
	BUILD_CGI_RESPONSE,
	SEND_RESPONSE,
	DISCONNECT
};

enum clientRequestType
{
	UNDEFINED,
	OTHER,
	MULTIPART,
	CHUNKED
};

extern bool	isSigInt;

void	sigIntHandler(int signal);
