CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I../../include
SRC = test_configparser.cpp
OBJS = $(SRC:.cpp=.o)

# List all required source files for linking
SRCS = ../../src/ConfigParser.cpp ../../src/parserProcessing.cpp

all: test_configparser

test_configparser: $(SRC) $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRC) $(SRCS) -o test_configparser

clean:
	rm -f *.o test_configparser

.PHONY: all clean
