# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/14 12:54:14 by vcaratti          #+#    #+#              #
#    Updated: 2026/02/14 12:54:54 by vcaratti         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME				= webserv

VPATH				= src

SRC_DIR				= src
SRCS				= main.cpp \
					  CgiRunner.cpp \
					  client_cgi.cpp \
					  ConfigParser.cpp \
					  DirectoryListing.cpp \
					  HttpPacket.cpp \
					  HttpRequest.cpp \
					  HttpRequestParser.cpp \
					  HttpResponse.cpp \
					  ParseFormData.cpp \
					  ResponseBuilder.cpp \
					  client.cpp \
					  clientTOrequest.cpp \
					  parserProcessing.cpp \
					  sending.cpp \
					  socket_poll.cpp \
					  Match_location.cpp \

INC_DIR				= include
INCLUDES			= -I$(INC_DIR)

BUILD_DIR			= build

OBJS				= $(addprefix $(BUILD_DIR)/, $(SRCS:%.cpp=%.o))
DEPS				= $(OBJS:.o=.d)

CC					= c++
CFLAGS				= -Wall -Wextra -Werror -g3 -O3 -std=c++98 -MMD

RM					= rm -rf



all: $(NAME)
#


$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
#


$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
#


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
#


run: all
	./$(NAME)
#

vrun: all
	valgrind --leak-check=full ./$(NAME)
#


clean:
	$(RM) $(BUILD_DIR)
#

fclean:
	$(RM) $(BUILD_DIR) $(NAME)
#

re:
	@$(MAKE) --no-print-directory fclean
	@$(MAKE) --no-print-directory all
#


-include $(DEPS)


.PHONY: all clean fclean re
