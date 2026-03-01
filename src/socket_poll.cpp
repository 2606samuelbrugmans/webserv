#include "webserv.hpp"
#include <cstring>
// create, bind and listen on a socket described by `config` and store the
// resulting file descriptor in both the ServerConfig and the given webserv
// state object so that the main loop can poll it.
void socketing(ServerConfig &config, webserv &server) {
    (void)server; // parameter currently unused, required by prototype

    int fd;
    fd = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
	if (fd < 0)
    	throw std::runtime_error("socket failed");
	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr)); //important to zero out the structure before filling it in,
	// otherwise we might have garbage data in there that causes bind to fail
	addr.sin_family = AF_INET; // IPv4
	addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
	addr.sin_port = htons(config.port);
	if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
    	throw std::runtime_error("bind failed");
	if (listen(fd, SOMAXCONN) < 0)
    	throw std::runtime_error("listen failed");
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl failed");
	config.fd = fd;
}

void polling(int fd, std::vector<pollfd>& connection) {
	//we add server fd to our list of fds to poll on, and then we will add client fds as they connect
	pollfd polled;
	polled.fd = fd;
	polled.events = POLLIN;
	polled.revents = 0;
	connection.push_back(polled);
}


bool isListeningSocket(const webserv& connection, int fd)
{ 
	// the same logic applies regardless of whether we use the old
	// Connection struct or the newer webserv; we just iterate the
	// configured servers and compare the listening descriptor.
	for (size_t i = 0; i < connection.servers.size(); ++i)
	{
		if (connection.servers[i].fd == fd)
			return true;
	}
	return false;
}