#pragma once

#include "webserv.hpp"

struct ListeningSocket {
    int fd;
    int port;
};

struct Connection {
    std::vector<pollfd> fds;
    std::map<int, Client> clients;
    std::vector<ListeningSocket> servers;
};
