/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRunner.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:48:17 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:48:33 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include "webserv.hpp"

// Helper to convert integer to string (C++98-compatible)
static std::string intToString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

// Helper to build CGI environment variables
std::vector<std::string> buildCgiEnv(/*const Location& location,*/ const ServerConfig& serverConfig, const std::string& scriptPath, const std::string& method, const std::string& query, const std::string& contentType, size_t contentLength) {
    std::vector<std::string> env;
    env.push_back("REQUEST_METHOD=" + method);
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    env.push_back("SCRIPT_NAME=" + scriptPath);
    env.push_back("SERVER_NAME=" + serverConfig.server_name);
    env.push_back("SERVER_PORT=" + (serverConfig.port ? intToString(serverConfig.port) : std::string("80")));
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("REDIRECT_STATUS=200"); // for PHP CGI compliance
    if (!query.empty())
        env.push_back("QUERY_STRING=" + query);
    if (!contentType.empty())
        env.push_back("CONTENT_TYPE=" + contentType);
    if (contentLength > 0)
        env.push_back("CONTENT_LENGTH=" + intToString(contentLength));
    // Add more variables as needed
    return env;
}

// Updated signature to accept ServerConfig
void runCgiScript(/*const Location& location,*/ const ServerConfig& serverConfig, 
    const std::string& scriptPath, const std::vector<std::string>& env, Client &client, webserv &connection) {
/*    int inPipe[2];
    int outPipe[2];
    if (pipe(inPipe) < 0 || pipe(outPipe) < 0)
        throw std::runtime_error("pipe() failed");
*/    
    /// we moved this back upstream to pipe and register the fds to monitor with poll for input and output
    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("fork() failed");

    if (pid == 0) {
        // Child process: setup stdin/stdout, exec
        close(client.get_cgi_handles().input_fds[1]);
        dup2(client.get_cgi_handles().input_fds[0], STDIN_FILENO);
        close(client.get_cgi_handles().input_fds[0]);

        close(client.get_cgi_handles().output_fds[0]);
        dup2(client.get_cgi_handles().output_fds[1], STDOUT_FILENO);
        close(client.get_cgi_handles().output_fds[1]);

        // Find interpreter from location.cgi_types and scriptPath extension
        std::string interpreter;
        size_t dot = scriptPath.find_last_of('.');
        if (dot != std::string::npos) {
            std::string ext = scriptPath.substr(dot);
            std::map<std::string, std::string>::const_iterator it = serverConfig.cgi_interpreters.find(ext);
            if (it != serverConfig.cgi_interpreters.end()) {
                interpreter = it->second;
            }
        }
        if (interpreter.empty())
            exit(127);

        // Build envp from env vector
        std::vector<char*> envp;
        for (size_t i = 0; i < env.size(); ++i)
            envp.push_back(const_cast<char*>(env[i].c_str()));
        envp.push_back(NULL);

        char* argv[3];
        argv[0] = const_cast<char*>(interpreter.c_str());
        argv[1] = const_cast<char*>(scriptPath.c_str());
        argv[2] = NULL;

        execve(interpreter.c_str(), argv, envp.data());
        perror("execve");
        exit(1);
    }

    // Parent process
    close(client.get_cgi_handles().input_fds[0]);
    close(client.get_cgi_handles().output_fds[1]);
    // Add CGI output fd to poll list to read CGI output when it's ready
    pollfd pfd;
    pfd.fd = client.get_cgi_handles().output_fds[0];
    pfd.events = POLLIN;
    pfd.revents = 0;
    connection.fds.push_back(pfd);
    // create fd_context entry without relying on C++11 initializer lists
    fd_context ctx;
    ctx.type = fd_context::CGI_OUTPUT;
    ctx.server_index = connection.fd_contexts[client.getClientFd()].server_index;
    ctx.client = &client;
    ctx.cgi_pid = pid;
    connection.fd_contexts[client.get_cgi_handles().output_fds[0]] = ctx;
     // Add CGI output fd to fd contexts with reference to client and CGI pid for later use when reading CGI output
    // Write input to child
    /*
    if (!input.empty()) {
        ssize_t nw = write(client.get_cgi_handles().input_fds[1], input.c_str(), input.size());
        (void)nw; // silence unused-result warning
    }
    */
    close(client.get_cgi_handles().input_fds[1]);

    // Read output from child (non-blocking)
    /*
    std::string output;
    char buf[4096];
    ssize_t bytesRead;

    int flags = fcntl(outPipe[0], F_GETFL, 0);
    fcntl(outPipe[0], F_SETFL, flags | O_NONBLOCK);

    while ((bytesRead = read(outPipe[0], buf, sizeof(buf))) > 0)
        output.append(buf, bytesRead);
    close(outPipe[0]);
    
    int status = 0;
    pid_t result = waitpid(pid, &status, WNOHANG);
    if (result < 0) {
        throw std::runtime_error("waitpid() failed");
    }
    */
    //return output;
}
