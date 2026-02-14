#include "webserv.hpp"
#include <cstring>

void socketing(Connection &connection) {

	ListeningSocket server;
	server.fd = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
	if (server.fd < 0)
    	throw std::runtime_error("socket failed");
	int opt = 1;
	setsockopt(server.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr)); //important to zero out the structure before filling it in,
	// otherwise we might have garbage data in there that causes bind to fail
	addr.sin_family = AF_INET; // IPv4
	addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
	addr.sin_port = htons(8080); // for now we hardcode the port, but this will be determined by config file later
	if (bind(server.fd, (sockaddr*)&addr, sizeof(addr)) < 0)
    	throw std::runtime_error("bind failed");
	listen(server.fd, SOMAXCONN);
	fcntl(server.fd, F_SETFL, O_NONBLOCK); //helps avoid blocking on accept
	connection.servers.push_back(server); // add server socket to our list of servers
}

void polling(int fd, Connection& connection) {

	//we add server fd to our list of fds to poll on, and then we will add client fds as they connect
	std::vector<pollfd> fds;
	pollfd polled;
	polled.fd = fd;
	polled.events = POLLIN;
	polled.revents = 0;
	fds.push_back(polled);
	connection.fds = fds;
}

bool isListeningSocket(const Connection& connection, int fd)
{
	// we check if the fd that woke up is one of our listening sockets, which means a new client is trying to connect
    for (size_t i = 0; i < connection.servers.size(); ++i)
    {
        if (connection.servers[i].fd == fd)
            return true;
    }
    return false;
}