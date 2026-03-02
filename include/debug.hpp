#pragma once

#include "webserv.hpp"


struct Location;
struct ServerConfig;


void printLocation(const Location& loc);
void printServerConfig(const ServerConfig& config);
void printAllConfigs(const std::vector<ServerConfig>& configs);