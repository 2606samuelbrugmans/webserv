/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRunner.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:42:33 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 13:03:25 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIRUNNER_HPP
#define CGIRUNNER_HPP

#include <string>
#include <vector>
#include "webserv.hpp"

// Runs a CGI script for a given Location and ServerConfig, using the request body as input. Returns the script's output.
void runCgiScript(/*const Location& location,*/ const ServerConfig& serverConfig, 
    const std::string& scriptPath, const std::vector<std::string>& env, Client &client, webserv &connection);
std::vector<std::string> buildCgiEnv(/*const Location& location,*/ const ServerConfig& serverConfig, const std::string& scriptPath, const std::string& method, const std::string& query, const std::string& contentType, size_t contentLength);
#endif // CGIRUNNER_HPP
