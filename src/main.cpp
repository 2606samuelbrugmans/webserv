/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:50:50 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:50:58 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "webserv.hpp"

inline std::string currentTimeString() {
    std::time_t now = std::time(NULL);
    std::tm     tm_now;
    localtime_r(&now, &tm_now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &tm_now);
    return std::string(buf);
}

void logRequest(const HttpRequest &req, const HttpResponse &res) {
    std::cout << "[" << colors::black << currentTimeString() << colors::reset << "] ";

    if (req.getMethod() == Http::Method::GET)
        std::cout << colors::green;
    else if (req.getMethod() == Http::Method::POST)
        std::cout << colors::yellow;
    else if (req.getMethod() == Http::Method::DELETE)
        std::cout << colors::red;
    else
        std::cout << colors::magenta;

    std::cout << Http::methodToString(req.getMethod()) << " " << colors::cyan << req.getPath()
              << colors::reset << " ";

    const int code = res.getStatusCode();
    if (code >= 200 && code < 300)
        std::cout << colors::green;
    else if (code >= 300 && code < 400)
        std::cout << colors::yellow;
    else if (code >= 400 && code < 500)
        std::cout << colors::red;
    else
        std::cout << colors::magenta;
    std::cout << code << colors::reset;

    std::cout << "\n";
}
/*
void initServer(std::vector<ServerConfig> &configs, const std::string &configFile) {
    
    
    ServerConfig config;
    std::set<Http::Method::Type> allowed_methods;
    allowed_methods.insert(Http::Method::GET);
    allowed_methods.insert(Http::Method::POST);
    allowed_methods.insert(Http::Method::DELETE);

    std::vector<std::string> cgi_types;

    config.error_pages_paths[Http::Status::Forbidden]           = "./www/error_pages/403.html";
    config.error_pages_paths[Http::Status::NotFound]            = "./www/error_pages/404.html";
    config.error_pages_paths[Http::Status::InternalServerError] = "./www/error_pages/500.html";

    // cgi_types.push_back("*.py");
    // cgi_types.push_back("*.php");

    Location loc1 = {allowed_methods, true, "/", "./www/", "index.html", "./www/upload", cgi_types};
    Location loc2 = {allowed_methods, true,           "/sub/images", "./www/alt/images/",
                     "index.html",    "./www/upload", cgi_types};

    config.locations.push_back(loc1);
    config.locations.push_back(loc2);
}
*/
std::string readHttpRequest(int client_fd) {
    // TODO: fix reading the request from a user

    std::string request;
    char        buffer[4096];

    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    usleep(10000);  // Random 10ms delay

    while (true) {
        ssize_t bytesRead = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) break;
        request.append(buffer, bytesRead);
    }
    return request;
}
/*

int main() {
    //////////////////////////////////////////////////
    // Simple demo server
    //////////////////////////////////////////////////

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "socket() failed\n";
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt() failed\n";
        close(server_fd);
        return 1;
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "bind() failed\n";
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "listen() failed\n";
        close(server_fd);
        return 1;
    }
    std::cout << "Test server running on http://localhost:8080" << "\n";

    //////////////////////////////////////////////////

    ServerConfig config;
    initServer(config);

    while (true) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            std::cerr << "accept() failed" << "\n";
            continue;
        }
        HttpRequest  req = HttpRequestParser::parse(readHttpRequest(client_fd));
        HttpResponse res = responseBuilder(config, req);
        logRequest(req, res);

        HttpResponse::sendResponse(client_fd, res);

        shutdown(client_fd, SHUT_WR);
        close(client_fd);
    }
    close(server_fd);
    return 0;
}
*/
void buildResponse(Client &client,  ServerConfig &config, webserv &connection) {
    // Example of building a response for a client
    HttpRequest req = buildRequest(client);
    HttpResponse res = responseBuilder(config, req, client, connection);
    client.response = res; // store the response in the client struct so that we can send it in the POLLOUT event in the main loop
    client.response.prepare(); // prepare the response (serialize it) before sending
    logRequest(req, res);
}
int main(int argc, char* argv[]) {

	if ( argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }
    webserv server;
    std::vector<ServerConfig> configs;
    try {
        configs = ConfigParser::parse(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config file: " << e.what() << '\n';
    }

    server.servers = configs;
    printAllConfigs(server.servers); // print all configs for debugging
    try {
        for (size_t i = 0; i < server.servers.size(); i++) {
            ServerConfig& server_config = server.servers[i];
            socketing(server_config, server);
            polling(server_config.fd, server.fds);
            fd_context ctx;
            ctx.type = fd_context::SERVER;
            ctx.server_index = static_cast<int>(i);
            ctx.client = NULL;
            ctx.cgi_pid = 0;
            server.fd_contexts[server_config.fd] = ctx; // add server socket to fd contexts
        }
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
    while (true)
    {
        poll(&server.fds[0], server.fds.size(), -1);
        for (size_t i = 0; i < server.fds.size(); i++)
        {
            pollfd p = server.fds[i]; // for clarity
            fd_context& current_ctx = server.fd_contexts[p.fd];// also for clarity
            if ( p.revents & (POLLERR | POLLHUP | POLLNVAL) )
            {
                std::cout << "POLLERR | POLLHUP | POLLNVAL event on fd: " << p.fd << std::endl;
                // manage error, hang up, or invalid request
            }
            else if (p.revents & POLLIN)
            {
                int cgi_erorr = 0;
                std::cout << "POLLIN event on fd: " << p.fd << std::endl;
                if (current_ctx.type == fd_context::CGI_INPUT && current_ctx.client->get_client_state() == PREPARING_CGI) // CGI input ready to write
                {
                    cgi_handles cgi = current_ctx.client->get_cgi_handles();
                    
                    int write_result = WriteToCgiInput(cgi);
                    if (write_result < 0) {
                        std::cerr << "Error writing to CGI input" << std::endl;
                    }
                    else if (write_result == 1) {
                        runCgiScript(/*location,*/ server.servers[current_ctx.server_index],
                         "", cgi.env, *current_ctx.client, server); // we will set the actual script path and env later in runCgiScript when we fork, for now we just want to set the client state to waiting for cgi output after writing the input
                    // we can write the request body to the CGI input fd here, but for now we will just set the client state to READING_CGI and then we will write the request body to the CGI input fd in the runCgiScript function before we fork, so that we can avoid blocking in case of large request body
                        current_ctx.client->set_client_state(READING_CGI);
                    }
                }
                else if (current_ctx.type == fd_context::SERVER)      // new client ?
                    acceptNewClient(server, p.fd);
                else if (current_ctx.type == fd_context::CLIENT && current_ctx.client->get_client_state() == READING) // already accepted client
                    current_ctx.client->receiveData(server, p.fd, server.servers[current_ctx.server_index]);
                else if (current_ctx.type == fd_context::CGI_OUTPUT && current_ctx.client->get_client_state() == READING_CGI) // cgi output ready to read
                    cgi_erorr = current_ctx.client->read_response_from_cgi(server, p.fd);
                if (current_ctx.type == fd_context::CLIENT && current_ctx.client->get_client_state() == READY_TO_PROCESS)
                    buildResponse(*current_ctx.client, server.servers[current_ctx.server_index], server);
                if (cgi_erorr == -1) // handle if problem not sure for now we'll figure it out
                {
                    std::cerr << "Error reading from CGI output\n";
                }
                else if (cgi_erorr == -2)
                {
                    std::cerr << "Error waiting for CGI process: " << strerror(errno) << std::endl;
                }
            }
            else if (p.revents & POLLOUT)
                if (current_ctx.type == fd_context::CLIENT && current_ctx.client->get_client_state() == READY_TO_PROCESS)
                    current_ctx.client->sendData(server, p.fd);
        }
    }
    /*
        Connection connection;
    ServerConfig& config = configs[0]; //temporary
	try {
		socketing(connection);
		polling(connection.servers[0].fd, connection);
		//there can be lots of servers, but for now we just have one
		//number of servers will be determined by config file
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	while (true)
	{
    	poll(&connection.fds[0], connection.fds.size(), -1);
    	for (size_t i = 0; i < connection.fds.size(); i++)
    	{
			pollfd p = connection.fds[i];
        if ( p.revents & (POLLERR | POLLHUP | POLLNVAL) )
        {
            std::cout << "POLLERR | POLLHUP | POLLNVAL event on fd: " << p.fd << std::endl;
            // manage error, hang up, or invalid request
        }
		else if (p.revents & POLLIN)
		{
            std::cout << "POLLIN event on fd: " << p.fd << std::endl;
    		if (isListeningSocket(connection, p.fd))      // new client ?
        		acceptNewClient(connection, p.fd);
    		else                              // already accepted client
        		connection.clients[p.fd].receiveData(connection, p.fd, config);
            if (p.revents & POLLOUT)
                buildResponse(connection.clients[p.fd], config);
            if (connection.clients[p.fd].get_client_state() == READING_CGI) // if client state is waiting for cgi, we want to read the cgi output and set the response for this client, and then we can set client state to ready to process so that we can send the response in the POLLOUT event in the main loop
                polling(connection.clients[p.fd].get_cgi_handles().output_fd, connection); // this is how we wait for cgireading
		}
		else if (p.revents & POLLOUT)
            if (connection.clients[p.fd].get_client_state() == READY_TO_PROCESS)
                connection.clients[p.fd].read_response_from_cgi(connection, p.fd, config);
            
            else // only send response if client state is ready to process, which means we have built the response for this client after receiving complete request
                connection.clients[p.fd].sendData(connection, p.fd, config);
		}
	}
    */
	/*
	while (true)
	{
    poll(all_descriptors);

    for each fd that woke up
        route event to correct object
	}
		*/
	return 0;
}