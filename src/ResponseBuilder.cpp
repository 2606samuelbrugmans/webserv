/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vcaratti <vcaratti@student.42belgium.be>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 12:51:54 by vcaratti          #+#    #+#             */
/*   Updated: 2026/02/14 12:52:05 by vcaratti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/stat.h>
#include <unistd.h>  // access

#include <fstream>
#include <sstream>
#include <string>

#include "ParseFormData.hpp"
#include "webserv.hpp"
#include "CgiRunner.hpp"

// Check if file exists
inline bool fileExists(const std::string &path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode);
}

// Check if dir exists
inline bool dirExists(const std::string &path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) return false;  // No access or doesn't exist
    return (info.st_mode & S_IFDIR) != 0;
}

// Helper function to read a file into a string
inline std::string readFileToString(const std::string &path) {
    std::ifstream      file(path.c_str(), std::ios::in | std::ios::binary);
    std::ostringstream ss;
    if (file) ss << file.rdbuf();
    return ss.str();
}

std::string buildErrorPage(ServerConfig &config, Http::Status::Code code) {
    if (config.error_pages_paths.find(code) != config.error_pages_paths.end()) {
        const std::string error_page_path = config.error_pages_paths[code];
        if (fileExists(error_page_path) && !access(error_page_path.c_str(), R_OK))
            return readFileToString(error_page_path);
    }

    std::ostringstream oss;

    oss << "<html><head><title>" << code << " " << Http::reasonPhrase(code)
        << "</title></head><body>";
    oss << "<center><span style=\"color: red;\"><h1>" << code << " " << Http::reasonPhrase(code)
        << "</h1></span></center>";

    return oss.str();
}

/**
 * @brief Joins two path components into a single path, handling slashes.
 *
 * @param root The first (root) part of the path.
 * @param tail The second (tail) part of the path.
 * @return std::string The combined path.
 */
std::string joinPaths(const std::string &root, const std::string &tail) {
    if (root.empty()) return tail;
    if (tail.empty()) return root;

    if (root[root.size() - 1] == '/' && tail[0] == '/')
        return root + tail.substr(1);
    else if (root[root.size() - 1] != '/' && tail[0] != '/')
        return root + "/" + tail;
    else
        return root + tail;
}

// Returns a iterator to the selected location requested by request_path MIGHT NEED TO IMPROVE LOGIC
std::vector<Location>::iterator chooseLocation(std::vector<Location> &locations,
                                               const std::string     &request_path,
                                               std::size_t           &longest_prefix_len) {
    std::vector<Location>::iterator longest_location = locations.end();

    longest_prefix_len = 0;
    for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); ++it) {
        const std::string &path = (*it).path;

        size_t j = 0;
        while (j < path.size() && j < request_path.size() && (path[j] == request_path[j])) ++j;

        if (j > longest_prefix_len && ((j + 1) >= path.size())) {//
            longest_prefix_len = j;
            longest_location   = it;
        }
    }
    return longest_location;
}

// Helper function that handles POST request
// returns (true, HttpResponse) if request was bad
static std::pair<bool, HttpResponse> handlePostUploads(ServerConfig   &config,
                                                       const Location &location, HttpRequest &req,
                                                       Http::Status::Code &successStatus) {
    if (location.upload_path.empty()) {
        // Forbidden(403) if upload not allowed
        return std::make_pair(true, HttpResponse(Http::Status::Forbidden,
                                                 buildErrorPage(config, Http::Status::Forbidden),
                                                 Http::ContentType::TEXT_HTML));
    }

    FormData data = parseMultipartFormData(req);

    // Checking files for same filenames first
    // TODO: Upload file with randomly generated filename end
    for (std::map<std::string, std::vector<UploadedFile> >::iterator it = data.files.begin();
         it != data.files.end(); ++it) {
        for (std::vector<UploadedFile>::iterator jt = it->second.begin(); jt != it->second.end();
             ++jt) {
            if (jt->filename.empty()) continue;

            std::string filePath = joinPaths(location.upload_path, jt->filename);
            if (fileExists(filePath))
                return std::make_pair(true,
                                      HttpResponse(Http::Status::Conflict,
                                                   buildErrorPage(config, Http::Status::Conflict),
                                                   Http::ContentType::TEXT_HTML));
        }
    }

    // Saving files
    for (std::map<std::string, std::vector<UploadedFile> >::iterator it = data.files.begin();
         it != data.files.end(); ++it) {
        for (std::vector<UploadedFile>::iterator jt = it->second.begin(); jt != it->second.end();
             ++jt) {
            if (jt->filename.empty()) continue;

            std::string   filePath = joinPaths(location.upload_path, jt->filename);
            std::ofstream outFile(filePath.c_str());
            if (!outFile.is_open()) continue;

            std::vector<unsigned char> &data = jt->data;

            outFile.write(reinterpret_cast<const char *>(data.data()), data.size());
            if (!outFile.is_open())
                return std::make_pair(
                    true, HttpResponse(Http::Status::InternalServerError,
                                       buildErrorPage(config, Http::Status::InternalServerError),
                                       Http::ContentType::TEXT_HTML));
            outFile.close();
        }
    }
    successStatus = Http::Status::Created;
    return std::make_pair(false, HttpResponse());
}

// Helper to build CGI environment for a request
static std::vector<std::string> buildCgiEnvForRequest(const Location& location, const ServerConfig& config, const HttpRequest& req, const std::string& scriptPath) {
    std::string method = methodToString( req.getMethod() );
    std::string query;
    std::string contentType;
    size_t contentLength = 0;
    // Extract query string if present
    std::string path = req.getPath();
    size_t qpos = path.find('?');
    if (qpos != std::string::npos)
        query = path.substr(qpos + 1);
    // Content-Type and Content-Length for POST
    if (method == "POST") {
        contentType = req.getHeader("Content-Type");
        contentLength = req.getBody().size();
    }
    (void)location; // Avoid unused parameter warning if location is not used in buildCgiEnv
    return buildCgiEnv(/*location, */config, scriptPath, method, query, contentType, contentLength);
}

HttpResponse responseBuilder(ServerConfig &config, HttpRequest &req, Client &client, webserv &connection) {
    std::string request_path = req.getPath();

    std::size_t                     longest_prefix;
    std::vector<Location>::iterator location_it =
        chooseLocation(config.locations, request_path, longest_prefix);

    if (location_it == config.locations.end())
        return HttpResponse(Http::Status::InternalServerError,
                            buildErrorPage(config, Http::Status::InternalServerError),
                            Http::ContentType::TEXT_HTML);

    Location   &location      = *location_it;
    std::string response_path = joinPaths(location.root, request_path.substr(longest_prefix));

    if (location.accepted_methods.find(req.getMethod()) == location.accepted_methods.end())
        return HttpResponse(Http::Status::MethodNotAllowed,
                            buildErrorPage(config, Http::Status::MethodNotAllowed),
                            Http::ContentType::TEXT_HTML);

    Http::Status::Code successStatus = Http::Status::OK;

    if (req.getMethod() == Http::Method::POST) {
        std::pair<bool, HttpResponse> postRes =
            handlePostUploads(config, location, req, successStatus);
        if (postRes.first) return postRes.second;
    }

    // Returning MovedPermanently(301) if request doesnt end with / and file doesn't exist
    // Example:  /dir  ->  /dir/, wont return index.html
    if (*request_path.rbegin() != '/' && !fileExists(response_path) &&
        dirExists(response_path + "/")) {
        HttpResponse res(Http::Status::MovedPermanently, "");
        res.setLocation(request_path + "/");
        return res;
    }

    if (dirExists(response_path)) {
        if (location.directory_listing) {
            // Directory Listing
            if (access(response_path.c_str(), R_OK))
                return HttpResponse(Http::Status::Forbidden,
                                    buildErrorPage(config, Http::Status::Forbidden),
                                    Http::ContentType::TEXT_HTML);

            return HttpResponse(Http::Status::OK, DirectoryListing(request_path, response_path),
                                Http::ContentType::TEXT_HTML);
        }
        // Example:  /dir/  ->  /dir/index.html
        response_path += location.default_index;
    }

    if (!fileExists(response_path))
        return HttpResponse(Http::Status::NotFound, buildErrorPage(config, Http::Status::NotFound),
                            Http::ContentType::TEXT_HTML);

    if (access(response_path.c_str(), R_OK))
        return HttpResponse(Http::Status::Forbidden,
                            buildErrorPage(config, Http::Status::Forbidden),
                            Http::ContentType::TEXT_HTML);

    // CGI handling
    //TODO: move to separate function, check if file is executable??, check if file is in cgi_types before checking for interpreter??
    size_t dot = response_path.find_last_of('.');
    if (dot != std::string::npos) {
        std::map<std::string, std::string>::const_iterator it = config.cgi_interpreters.end();
        std::string ext = response_path.substr(dot);
        for (size_t i = 0; i < location.cgi_types.size(); ++i) {
            if (location.cgi_types[i] == ext)
            {
                it = config.cgi_interpreters.find(ext);
                break;
            }
        }
        if (it == config.cgi_interpreters.end()) {
            return HttpResponse(Http::Status::InternalServerError,
                    buildErrorPage(config, Http::Status::NotImplemented),
                    Http::ContentType::TEXT_HTML);
        }
        //CGI environment (minimal for now)
        // TODO: Add more CGI environment variables as needed
        std::vector<std::string> env = buildCgiEnvForRequest(location, config, req, response_path);
        std::string cgi_input;
        const std::vector<unsigned char>& bodyVec = req.getBody();
        if (!bodyVec.empty())
            cgi_input.assign(reinterpret_cast<const char*>(&bodyVec[0]), bodyVec.size());
        //how do i check if there is an input
        //here we should create the cgi fd context
        int inPipe[2];
        int outPipe[2];
        if (pipe(inPipe) < 0 || pipe(outPipe) < 0)
            throw std::runtime_error("pipe() failed");
        client.set_cgi_handles(0, inPipe, outPipe, cgi_input, env); // we will set the actual CGI pid later in runCgiScript when we fork
         // we will set the actual CGI pid later in runCgiScript when we fork        
        if (client.getContentLength() > 0)
        {
            fd_context ctx;
            ctx.type = fd_context::CGI_INPUT;
            ctx.server_index = connection.fd_contexts[client.getClientFd()].server_index;
            ctx.client = &client;
            ctx.cgi_pid = 0; // will be set in runCgiScript
            connection.fd_contexts[inPipe[1]] = ctx;
            client.set_client_state(PREPARING_CGI);
            return HttpResponse(Http::Status::OK, "Preparing CGI...", Http::ContentType::TEXT_PLAIN);
        }
        client.set_client_state(READING_CGI); // set client state to waiting for cgi, to not block
        runCgiScript(/*location,*/ config, response_path, env, client, connection);
        HttpResponse waiting;
        /*catch (const std::exception& e)
        {
            std::cerr << "CGI execution error: " << e.what() << std::endl; //ALLOWED to cerr??
            return HttpResponse(Http::Status::InternalServerError,
                    buildErrorPage(config, Http::Status::InternalServerError),
                    Http::ContentType::TEXT_HTML);
            }
        */
            // Return CGI output as HTTP response (assume script outputs full HTTP headers+body)
        return waiting; // we will read the actual CGI output in the main loop when we see that client state is READING_CGI, and then we can set the response for this client and set client state to READY_TO_PROCESS so that we can send the response in the POLLOUT event in the main loop
        }
    return HttpResponse(successStatus, readFileToString(response_path),
                        Http::contentTypeFromFile(response_path));
}
