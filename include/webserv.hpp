/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:47:56 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:48:07 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>


#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/poll.h>
#include "Http.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponse.hpp"
#include "colors.hpp"
#include "Client.hpp"
#include "Connection.hpp"
#include "ConfigParser.hpp"
#include "CgiRunner.hpp"
#include <cerrno>

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

struct Location {
    std::set<Http::Method::Type> accepted_methods;
    bool                         directory_listing;
    std::string                  path;  // Url prefix
    std::string                  root;  // Filesystem root
    std::string                  default_index;
    std::string                  upload_path;
    std::vector<std::string>     cgi_types;
    std::size_t                  body_size;
};

struct ServerConfig {
    std::string                               server_name;
    std::string                               interface;
    unsigned short                            port;
    std::map<Http::Status::Code, std::string> error_pages_paths;
    std::vector<Location>                     locations;
    std::map<std::string, std::string>        cgi_interpreters;
    int fd; // file descriptor for the server socket
};
/*
struct Connection {
    std::vector<pollfd> fds;
    std::map<int, Client> clients; // Map client fd to Client struct
    std::vector<ServerConfig> servers; // We can have multiple servers (different ports or server_names)
};
*/
// context information associated with a file descriptor
// (used by the poll loop to figure out what kind of
// object owns the descriptor)
struct fd_context {
    enum Type { SERVER, CLIENT, CGI_OUTPUT, CGI_INPUT} type;
    int server_index; // index in webserv.servers; indicates which server a client/cgi belongs to
    Client* client;   // pointer to Client object when type == CLIENT or CGI_OUTPUT
    pid_t cgi_pid;    // PID of the CGI process (redundant with client but handy)
};

// central state object used throughout the program
struct webserv {
    std::map<int, fd_context> fd_contexts; // Map fd -> context (server socket, client socket or CGI output)
    std::vector<ServerConfig> servers;     // server configurations (one per listening port/name)
    std::vector<pollfd> fds;               // pollfd array for all fds being monitored
};
HttpRequest buildRequest(const Client& client);
Http::Method::Type stringToMethod(const std::string& m);
int match_location(const ServerConfig &config, std::string request_path);

// builds a complete HttpResponse based on the server configuration, the
// parsed request and the client state.  The connection object is passed in
// because CGI execution may need to register additional fds.
HttpResponse responseBuilder(ServerConfig &config,
                             HttpRequest &req,
                             Client &client,
                             webserv &connection);
std::string  DirectoryListing(const std::string &request_path, const std::string &resposne_path);
void buildResponse(Client &client,  ServerConfig &config, webserv &connection);
void acceptNewClient(webserv &server, int server_fd);
#endif  // WEBSERV_HPP
