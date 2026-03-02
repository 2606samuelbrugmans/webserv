#pragma once
#include "HttpResponse.hpp"
#include <string>
#include <map>
#include <vector>
#include <poll.h> // for pollfd

// forward declarations
struct webserv;
struct ServerConfig;

struct RequestLine {
	std::string method;
	std::string url;
	std::string version;
};

enum ClientState {
	READING,
	WRITING,
	CLOSED,
	READING_CGI,
	READY_TO_PROCESS,
	PREPARING_CGI
};

struct cgi_handles {
	int input_fds[2];  // For writing request body to CGI
	int output_fds[2]; // For reading CGI output
	pid_t pid;     // Doesn't seem relevant for now epsecially since it already is in the fd_context
	std::string input_data; // The CGI input data to be written to the input pipe
	std::vector<std::string> env; 
};
// utility functions that operate on the webserv state
// (previously used a separate Connection type, the code now
// works directly on the webserv structure)
bool isListeningSocket(const webserv& connection, int fd);
void socketing(ServerConfig &config, webserv &server);
void polling(int fd, std::vector<pollfd>& connection);
void acceptNewClient(webserv &connection, int fd);
int WriteToCgiInput(cgi_handles &cgi);
struct fileconfig {};

class Client {
public:
	Client();
	Client(int fd);
	int getClientFd() const;
	std::string getRecvBuffer() const;
	bool getHeadersParsed() const;
	int getContentLength() const;
	void setRecvBuffer(const std::string& buffer);
	void setHeadersParsed(bool parsed);
	void setContentLength(int length);
	void setHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& b);
	void set_cgi_handles(pid_t pid, int input_fd[2], int output_fd[2], std::string cgi_input, std::vector<std::string> env);
	cgi_handles get_cgi_handles() const;
	void clearHeaders();
	int read_response_from_cgi(webserv &connection, int fd);

	void reset();
	void receiveData(webserv &connection, int fd, ServerConfig &config);
	void sendData(webserv &connection, int fd);
	bool getRequestParsed() const;
	void setRequestParsed(bool parsed);
	void set_location_index(int index) { location_index = index; }
	int get_location_index() const { return location_index; }
	ClientState get_client_state() const;
	void set_client_state(ClientState state);
	RequestLine request_line;
	std::map<std::string, std::string> headers; // Parsed headers will be stored here
	HttpResponse response; // We can store the response here after building it, so that we can send it in the POLLOUT event in the main loop
	std::string body;
	~Client();
private:
	cgi_handles cgi; // Store CGI process info if this client is running a CGI script
	ClientState state;
	int location_index; // index of th
	int client_fd;
	std::string recv_buffer;
	bool request_parsed;
	bool headers_parsed;
	int content_length;
	size_t header_end;      // index of "\r\n\r\n"
	size_t expected_size;   // total request size (headers + body)
	size_t bytes_sent; // Track how many bytes of the response have been sent, for handling partial sends
};

void parseHeaders(Client &client, const std::string& headerPart);