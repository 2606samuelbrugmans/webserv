#include "webserv.hpp"


#include "debug.hpp"
void printLocation(const Location& loc)
{
    std::cout << "    Location:\n";
    std::cout << "      Path: " << loc.path << "\n";
    std::cout << "      Root: " << loc.root << "\n";
    std::cout << "      Default Index: " << loc.default_index << "\n";
    std::cout << "      Upload Path: " << loc.upload_path << "\n";
    std::cout << "      Directory Listing: "
              << (loc.directory_listing ? "on" : "off") << "\n";
    std::cout << "      Body Size: " << loc.body_size << "\n";

    std::cout << "      Accepted Methods: ";
    for (std::set<Http::Method::Type>::const_iterator it = loc.accepted_methods.begin();
         it != loc.accepted_methods.end(); ++it)
    {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    std::cout << "      CGI Types: ";
    for (std::vector<std::string>::const_iterator it = loc.cgi_types.begin();
         it != loc.cgi_types.end(); ++it)
    {
        std::cout << *it << " ";
    }
    std::cout << "\n";
}

void printServerConfig(const ServerConfig& config)
{
    std::cout << "========================================\n";
    std::cout << "Server Name: " << config.server_name << "\n";
    std::cout << "Interface: " << config.interface << "\n";
    std::cout << "Port: " << config.port << "\n";
    std::cout << "Socket FD: " << config.fd << "\n";

    std::cout << "  Error Pages:\n";
    for (std::map<Http::Status::Code, std::string>::const_iterator it =
             config.error_pages_paths.begin();
         it != config.error_pages_paths.end(); ++it)
    {
        std::cout << "    " << it->first << " -> " << it->second << "\n";
    }

    std::cout << "  CGI Interpreters:\n";
    for (std::map<std::string, std::string>::const_iterator it =
             config.cgi_interpreters.begin();
         it != config.cgi_interpreters.end(); ++it)
    {
        std::cout << "    " << it->first << " -> " << it->second << "\n";
    }

    std::cout << "  Locations:\n";
    for (std::vector<Location>::const_iterator it = config.locations.begin();
         it != config.locations.end(); ++it)
    {
        printLocation(*it);
    }

    std::cout << "========================================\n";
}

void printAllConfigs(const std::vector<ServerConfig>& configs)
{
    for (std::vector<ServerConfig>::const_iterator it = configs.begin();
         it != configs.end(); ++it)
    {
        printServerConfig(*it);
    }
}