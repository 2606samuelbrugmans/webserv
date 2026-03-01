/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:48:35 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 13:22:55 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "parserProcessing.hpp"

static std::vector<std::string> extractServerBlocks(std::string& conf) {
	std::vector<std::string> server_blocks;
	size_t found = conf.find("server");

	while (found != std::string::npos) {
		size_t start = conf.find('{', found);
		if (start == std::string::npos)
			break;

		int brace_count = 1;
		size_t end = start + 1;

		while (end < conf.size() && brace_count > 0)
		{
			if (conf[end] == '{')
				brace_count++;
			else if (conf[end] == '}')
				brace_count--;
			end++;
		}

		if (brace_count == 0)
		{
			std::string block = conf.substr(found, end - found);
			server_blocks.push_back(block);
			conf.erase(found, end - found);
			found = conf.find("server");
		}
		else
			throw std::runtime_error("Unmatched braces in server block");
	}

	return server_blocks;
}

static std::vector<std::string> extractLocationBlocks(std::string& serverBlock) {
	std::vector<std::string> locations;
	size_t pos = 0;
	size_t found = serverBlock.find("location", pos);

	while (found != std::string::npos) {
		size_t start = serverBlock.find('{', found);
		if (start == std::string::npos)
			break;

		int brace_count = 1;
		size_t end = start + 1;

		while (end < serverBlock.size() && brace_count > 0) {
			if (serverBlock[end] == '{')
				brace_count++;
			else if (serverBlock[end] == '}')
				brace_count--;
			end++;
		}

		if (brace_count == 0) {
			std::string block = serverBlock.substr(found, end - found);
			locations.push_back(block);
			serverBlock.erase(found, end - found);
			found = serverBlock.find("location", pos);
		} else {
			throw std::runtime_error("Unmatched braces in location block");
		}
	}

	return locations;
}

static std::vector<std::string> extractCgiBlocks(std::string& serverBlock) {
	std::vector<std::string> cgiBlocks;
	size_t pos = 0;
	size_t found = serverBlock.find("cgi", pos);

	while (found != std::string::npos) {
		size_t start = serverBlock.find('{', found);
		if (start == std::string::npos)
			break;

		int brace_count = 1;
		size_t end = start + 1;

		while (end < serverBlock.size() && brace_count > 0) {
			if (serverBlock[end] == '{')
				brace_count++;
			else if (serverBlock[end] == '}')
				brace_count--;
			end++;
		}

		if (brace_count == 0) {
			std::string block = serverBlock.substr(found, end - found);
			cgiBlocks.push_back(block);
			serverBlock.erase(found, end - found);
			// After erasing, don't advance pos, just search from the same position
			found = serverBlock.find("cgi", pos);
		} else {
			throw std::runtime_error("Unmatched braces in cgi block");
		}
	}

	return cgiBlocks;
}

static std::vector<std::string> extractDirectives(const std::string& block) {
	std::vector<std::string> directives;
	std::istringstream iss(block);
	std::string line;

	while (std::getline(iss, line)) {
		size_t first = line.find_first_not_of(" \t\n\r");
		size_t last = line.find_last_not_of(" \t\n\r");
		if (first != std::string::npos && last != std::string::npos)
			line = line.substr(first, last - first + 1);
		else
			continue;

		// Ignore block starts/ends and nested blocks
		if (line.find("{") != std::string::npos || line.find("}") != std::string::npos ||
			line.find("location") == 0 || line.find("cgi ") == 0 || line.find("cgi{") == 0)
			continue;

		directives.push_back(line);
	}

	return directives;
}

// Validates and prepares the ServerConfig vector for execution
void validateAndPrepareServerConfigs(std::vector<ServerConfig>& servers) {
    for (size_t i = 0; i < servers.size(); ++i) {
        ServerConfig& server = servers[i];
        // Check listen
        if (server.interface.empty() && server.port == 0)
            throw std::runtime_error("Server block missing 'listen' directive");
        // Default server_name
        if (server.server_name.empty())
            server.server_name = "localhost";
        // Default error pages (optional, can be left empty)
        // Check at least one location
        if (server.locations.empty())
            throw std::runtime_error("Server block missing at least one 'location' block");
        for (size_t j = 0; j < server.locations.size(); ++j) {
            Location& loc = server.locations[j];
            // Path required
            if (loc.path.empty())
                throw std::runtime_error("Location block missing path");
            // Root required
            if (loc.root.empty())
                throw std::runtime_error("Location block missing 'root' directive");
            // Default index
            if (loc.default_index.empty())
                loc.default_index = "index.html";
            // Default directory_listing
            // (false by default in struct)
            // Default upload_path
            // (empty string = uploads not allowed)
            // Default allowed_methods
            if (loc.accepted_methods.empty())
                loc.accepted_methods.insert(Http::Method::GET);
            // Default cgi_types: empty = no CGI
            // If cgi_types present, check interpreter
            for (size_t k = 0; k < loc.cgi_types.size(); ++k) {
                const std::string& ext = loc.cgi_types[k];
                if (server.cgi_interpreters.find(ext) == server.cgi_interpreters.end())
                    throw std::runtime_error(std::string("CGI extension '") + ext + "' in location '" + loc.path + "' has no interpreter configured");
            }
        }
    }
}

std::vector<ServerConfig> ConfigParser::parse(const std::string& filepath) { //throw
	std::stringstream buffer;
	std::ifstream file(filepath.c_str());
	std::vector<ServerConfig> servers;
	std::string conf;
	size_t global_max_body_size = std::atol("0"); // default to 0 (unlimited) if not set in global directives

	if (!file.is_open())
		throw std::runtime_error("Could not open config file: " + filepath);
	buffer << file.rdbuf();
	conf = buffer.str();

	std::vector<std::string> serverBlocks = extractServerBlocks(conf);
	std::vector<std::string> globalDirectives = extractDirectives(conf);
	processGlobalDirectives(globalDirectives, global_max_body_size);

	for (size_t i = 0; i < serverBlocks.size(); ++i) {
		ServerConfig serverConfig;
		size_t server_max_body_size = global_max_body_size; //default to global, can be overridden by server directive

		std::vector<std::string> locationBlocks =	extractLocationBlocks(serverBlocks[i]);
		//std::cout << "left in block:" << serverBlocks[i] << std::endl;
		//for (size_t j = 0; j < locationBlocks.size(); ++j) {
		//	std::cout << "location block:" << locationBlocks[j] << std::endl;
		//}
		std::vector<std::string> cgiBlocks =		extractCgiBlocks(serverBlocks[i]);
		std::vector<std::string> serverDirectives =	extractDirectives(serverBlocks[i]);
		processServerDirectives(serverDirectives, serverConfig, server_max_body_size);

		for (size_t j = 0; j < locationBlocks.size(); ++j) {
			Location loc;
			loc.body_size = server_max_body_size; // default to server, can be overridden by location directive
			// Cleanly extract the path from the block header
			std::istringstream iss(locationBlocks[j]);
			std::string token, path;
			iss >> token >> path;
			loc.path = path;
			std::vector<std::string> locationDirectives = extractDirectives(locationBlocks[j]);
			processLocationDirectives(locationDirectives, loc);
			serverConfig.locations.push_back(loc);
		}

		for (size_t j = 0; j < cgiBlocks.size(); ++j) {
			std::vector<std::string> cgiDirectives = extractDirectives(cgiBlocks[j]);
			processCgiDirectives(cgiDirectives, serverConfig);
		}
		servers.push_back(serverConfig);
	}
	validateAndPrepareServerConfigs(servers);
	return servers;
}



