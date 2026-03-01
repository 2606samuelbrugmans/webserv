/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parserProcessing.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:51:17 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 13:22:27 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parserProcessing.hpp"
#include "ConfigParser.hpp"
#include <sstream>
#include <iostream>

static void splitDirective(const std::string& directive, std::string& key, std::string& value) {
	size_t space = directive.find(' ');
	if (space != std::string::npos)
	{
		key = directive.substr(0, space);
		value = directive.substr(space + 1);
	}
	else
	{
		key = directive;
		value = "";
		std::cerr << "[WARN] Directive '" << key << "' has no value." << std::endl;
	}
	if (!value.empty() && *(value.rbegin()) == ';')
		value.erase(value.size() - 1);
}

void processServerDirectives(const std::vector<std::string>& directives, ServerConfig& serverConfig, size_t& server_max_body_size ) {
	for (size_t i = 0; i < directives.size(); ++i)
	{
		std::string key, value;
		splitDirective(directives[i], key, value);
		std::cout << "Processing server directive: " << key << " with value: " << value << std::endl;
		if (key == "listen")
		{
			size_t colon = value.find(":");
			if (colon != std::string::npos)
			{
				serverConfig.interface = value.substr(0, colon);
				serverConfig.port = static_cast<unsigned short>(std::atoi(value.substr(colon + 1).c_str()));
			}
		}
		else if (key == "server_name")
			serverConfig.server_name = value;
		else if (key == "error_page")
		{
			std::istringstream iss(value);
			int code;
			std::string path;
			iss >> code >> path;
			serverConfig.error_pages_paths[static_cast<Http::Status::Code>(code)] = path;
		}
		else if (key == "client_max_body_size")
			server_max_body_size = std::atol(value.c_str());
		else
			std::cerr << "[WARN] Unhandled server directive: " << key << std::endl;
	}
}


void processLocationDirectives(std::vector<std::string> directives, Location& loc)
{
	for ( std::vector<std::string>::iterator it = directives.begin(); it != directives.end(); ++it) {
		std::cout << "--Location directive: " << *it << std::endl;
	}
	for (size_t j = 0; j < directives.size(); ++j)
	{
		std::string key, value;
		splitDirective(directives[j], key, value);
		std::cout << "Processing location directive: " << key << " with value: " << value << std::endl;
		if (key == "root")
			loc.root = value;
		else if (key == "index")
			loc.default_index = value;
		else if (key == "autoindex")
			loc.directory_listing = (value == "on"); 
		else if (key == "upload_path")
			loc.upload_path = value;
		else if (key == "allowed_methods")
		{
			std::istringstream iss(value);
			std::string method;
			while (iss >> method)
			{
				if (method == "GET")
					loc.accepted_methods.insert(Http::Method::GET);
				else if (method == "POST")
					loc.accepted_methods.insert(Http::Method::POST);
				else if (method == "DELETE")
					loc.accepted_methods.insert(Http::Method::DELETE);
				else
					std::cerr << "[WARN] Unrecognized HTTP method in allowed_methods: " << method << std::endl;
			}
		}
		else if (key == "cgi_extensions")
		{
			std::istringstream iss(value);
			std::string ext;
			while (iss >> ext)
				loc.cgi_types.push_back(ext);
		}
		else if (key == "client_max_body_size")
		{
			loc.body_size = std::atol(value.c_str());
		}
		else
		{
			std::cerr << "[WARN] Unhandled location directive: " << key << std::endl;
		}
	}
}

void processGlobalDirectives(const std::vector<std::string>& directives, size_t& global_max_body_size) {
	for (size_t i = 0; i < directives.size(); ++i) {
		std::string key, value;
		splitDirective(directives[i], key, value);
		std::cout << "Processing global directive: " << key << " with value: " << value << std::endl;
		if (key == "client_max_body_size") {
			global_max_body_size = std::atol(value.c_str());
		} else {
			std::cerr << "[WARN] Unhandled global directive: " << key << std::endl;
		}
	}
}

void processCgiDirectives(const std::vector<std::string>& directives, ServerConfig& serverConfig)
{
    std::string current_ext;
    std::string current_path;
    for (size_t i = 0; i < directives.size(); ++i)
    {
        std::string key, value;
        splitDirective(directives[i], key, value);
        std::cout << "Processing CGI directive: " << key << " with value: " << value << std::endl;
        if (key == "extension") {
            current_ext = value;   
        } else if (key == "path") {
            current_path = value;
        } else {
			std::cerr << "[WARN] Unhandled CGI directive: " << key << std::endl;
		}
        // If both extension and path are set, add to map and reset
        if (!current_ext.empty() && !current_path.empty()) {
            serverConfig.cgi_interpreters[current_ext] = current_path;
            current_ext.clear();
            current_path.clear();
        }
    }
}
