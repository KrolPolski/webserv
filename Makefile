#* *************************************************************************** *#
#*                                                                             *#
#*  $$\      $$\           $$\                                                 *#
#*  $$ | $\  $$ |          $$ |                                                *#
#*  $$ |$$$\ $$ | $$$$$$\  $$$$$$$\   $$$$$$$\  $$$$$$\   $$$$$$\ $$\    $$\   *#
#*  $$ $$ $$\$$ |$$  __$$\ $$  __$$\ $$  _____|$$  __$$\ $$  __$$\\$$\  $$  |  *#
#*  $$$$  _$$$$ |$$$$$$$$ |$$ |  $$ |\$$$$$$\  $$$$$$$$ |$$ |  \__|\$$\$$  /   *#
#*  $$$  / \$$$ |$$   ____|$$ |  $$ | \____$$\ $$   ____|$$ |       \$$$  /    *#
#*  $$  /   \$$ |\$$$$$$$\ $$$$$$$  |$$$$$$$  |\$$$$$$$\ $$ |        \$  /     *#
#*  \__/     \__| \_______|\_______/ \_______/  \_______|\__|         \_/      *#
#*                                                                             *#
#*   By: Panu Kangas, Ryan Boudwin, Patrik Lång                                *#
#*                                                                             *#
#* *************************************************************************** *#

NAME = webserv

COMP = c++

CFLAGS = -Wall -Wextra -Werror -std=c++20

RED = \033[31m
GREEN = \033[32m
RESET = \033[0m

# Paths
SRCS_PATH = srcs/
INCLUDES_PATH = includes/
OBJS_PATH = objs/

# Source Files
SRCS = $(addprefix $(SRCS_PATH), \
		main.cpp \
		ConnectionHandler.cpp \
		ResponseHandler.cpp \
		ConfigurationHandler.cpp \
		CgiHandler.cpp \
		signalHandling.cpp \
		requestParsing.cpp \
		Logger.cpp \
		URLhandler.cpp)

# Object Files
OBJS = $(addprefix $(OBJS_PATH), $(notdir $(SRCS:.cpp=.o)))

# Build Target
all: $(NAME)

$(NAME): $(OBJS_PATH) $(OBJS)
	@$(COMP) $(CFLAGS) -I$(INCLUDES_PATH) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(NAME) creation successful!$(RESET)"

# Compile .cpp into .o files
$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp | $(OBJS_PATH)
	@$(COMP) $(CFLAGS) -I$(INCLUDES_PATH) -c $< -o $@

# Create obj directory if it doesn’t exist
$(OBJS_PATH):
	@mkdir -p $(OBJS_PATH)

# Cleanup
clean:
	@rm -rf $(OBJS_PATH)
	@echo "$(RED)Object files removed!$(RESET)"

fclean: clean
	@rm -rf $(NAME)
	@rm -rf logfile.log
	@echo "$(RED)Full cleanup successful!$(RESET)"

re: fclean all

.PHONY: all clean fclean re
