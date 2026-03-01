/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_configparser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:52:42 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:52:54 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include <iostream>
#include <vector>
#include <iomanip>

void printServerConfig(const ServerConfig& config) {
    std::cout << "Server name: " << config.server_name << std::endl;
    std::cout << "Interface: " << config.interface << std::endl;
    std::cout << "Port: " << config.port << std::endl;
    std::cout << "Error pages:" << std::endl;
    for (std::map<Http::Status::Code, std::string>::const_iterator it = config.error_pages_paths.begin(); it != config.error_pages_paths.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
    std::cout << "CGI interpreters:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = config.cgi_interpreters.begin(); it != config.cgi_interpreters.end(); ++it) {
        std::cout << "  " << it->first << " => " << it->second << std::endl;
    }
    std::cout << "Locations:" << std::endl;
    for (size_t i = 0; i < config.locations.size(); ++i) {
        const Location& loc = config.locations[i];
        std::cout << "  Path: " << loc.path << std::endl;
        std::cout << "    Root: " << loc.root << std::endl;
        std::cout << "    Index: " << loc.default_index << std::endl;
        std::cout << "    Directory listing: " << (loc.directory_listing ? "on" : "off") << std::endl;
        std::cout << "    Upload path: " << loc.upload_path << std::endl;
        std::cout << "    Accepted methods: ";
        for (std::set<Http::Method::Type>::const_iterator mit = loc.accepted_methods.begin(); mit != loc.accepted_methods.end(); ++mit) {
            std::cout << methodToString(*mit) << " ";
        }
        std::cout << std::endl;
        std::cout << "    CGI types: ";
        for (size_t j = 0; j < loc.cgi_types.size(); ++j) {
            std::cout << loc.cgi_types[j] << " ";
        }
        std::cout << "\n    Body size: " << loc.body_size << std::endl;

        std::cout << std::endl;
    }
}

int main() {
    std::string config_dir = "configs/";
    std::vector<std::string> files;
    files.push_back("webserv.conf");
    //files.push_back("missing_bracket.conf");
    //files.push_back("empty_block.conf");
    //files.push_back("no_block_name.conf");
    //files.push_back("single_word_directive.conf");
    //files.push_back("no_directive_value.conf");
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "\n==== Parsing: " << files[i] << " ====" << std::endl;
        try {
            std::vector<ServerConfig> servers = ConfigParser::parse(config_dir + files[i]);
            std::cout << "Parsed " << servers.size() << " server blocks." << std::endl;
            for (size_t j = 0; j < servers.size(); ++j) {
                std::cout << "\nServer block " << j << ":" << std::endl;
                printServerConfig(servers[j]);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}
