#pragma once

#include <fcntl.h>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <poll.h>
#include <map>
#include <string>
#include <unistd.h>

struct Connection;

bool isListeningSocket(const Connection& connection, int fd);
void socketing(Connection &connection);
void polling(int server_fd, Connection& connection);
void receiveData(Connection &connection, int fd);
void acceptNewClient(Connection &connection, int fd);
struct fileconfig {
};


class Client {
public:
	Client(int fd) : client_fd(fd) {};
	int getClientFd() const;
	std::string getRecvBuffer() const;
	bool getHeadersParsed() const;
	int getContentLength() const;
	void setRecvBuffer(const std::string& buffer);
	void setHeadersParsed(bool parsed);
	void setContentLength(int length);
	~Client();
private:
	int client_fd;
	std::string recv_buffer;
	bool headers_parsed = false;
	int content_length = 0;

	//Here bellow parsing data
	//you don't have to use these, but they might be useful for building the response
	//because we need to save time on polling, we want to avoid doing extra work in the main loop,
	//so we can parse headers and build response in the receiveData function, and then just send the response
	//instead of waiting for POLLOUT to parse headers and build response, which would be inefficient
	std::string send_buffer; // Buffer for outgoing data 
	std::map<std::string, std::string> headers; // Parsed headers will be stored here
};
struct ListeningSocket {
    int fd;
    int port;
};

struct Connection {
    std::vector<pollfd> fds;
    std::map<int, Client> clients;
    std::vector<ListeningSocket> servers;
	fileconfig fileconfig;
};
