#include "webserv.hpp"
#include <cerrno>

Client::Client()
    : cgi(), state(READING), location_index(-1), client_fd(-1),
      recv_buffer(""), request_parsed(false), headers_parsed(false),
      content_length(0), header_end(0), expected_size(0), bytes_sent(0) {
    // Initialize client state here
}

Client::Client(int fd)
    : cgi(), state(READING), location_index(-1), client_fd(fd),
      recv_buffer(""), request_parsed(false), headers_parsed(false),
      content_length(0), header_end(0), expected_size(0), bytes_sent(0) {
    // Initialize client state here
    this->client_fd = fd;
}
Client::~Client() {
	// Clean up client state here
}
int Client::getClientFd() const {
	return client_fd;
}
std::string Client::getRecvBuffer() const {
	return recv_buffer;
}
bool 	Client::getHeadersParsed() const {
	return headers_parsed;
}
int Client::getContentLength() const {
	return content_length;
}
void Client::setRecvBuffer(const std::string& buffer) {
	recv_buffer = buffer;
}
void Client::setHeadersParsed(bool parsed) {
	headers_parsed = parsed;
}
void Client::setContentLength(int length) {
	content_length = length;
}
void Client::setHeader(const std::string& key, const std::string& value) {
	headers[key] = value;
}

void Client::setBody(const std::string& b) {
    body = b;
}

void Client::clearHeaders() {
    headers.clear();
}

void acceptNewClient(webserv &connection, int fd)
{
    while (true) // one or more clients may be waiting to connect
    {
        sockaddr_in addr;
        socklen_t len = sizeof(addr);

        int client_fd = accept(fd, (sockaddr*)&addr, &len);
        if (client_fd < 0) // do we still have clients ? and is there any issue on the server side ?
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) //clients to accept ?
                break; // no more clients to accept, exit loop
            return; // negative fd equal error if not EAGAIN or EWOULDBLOCK
        } // might want to log error for the evaluation once we're finishing this project
        Client* new_client = new Client(client_fd); // don't forget to delete the client and close the fds
        fcntl(client_fd, F_SETFL, O_NONBLOCK); //avoid blocking on this client socket
            polling(client_fd, connection.fds); // add client fd to polling list
            // can't use C++11 initializer list under -std=c++98, build a struct
            fd_context ctx;
            ctx.type = fd_context::CLIENT;
            ctx.server_index = -1;
            ctx.client = new_client;
            ctx.cgi_pid = 0;
            connection.fd_contexts[client_fd] = ctx; // add client fd to fd contexts

        //std::cout << "New client with fd: " << client_fd << std::endl;
    }
}
void parseHeaders(Client &client, const std::string& headerPart)
{
	size_t pos = 0;
	while (pos < headerPart.size())
	{
		size_t endOfLine = headerPart.find("\r\n", pos);
		if (endOfLine == std::string::npos)
			break;

		std::string line = headerPart.substr(pos, endOfLine - pos);
		pos = endOfLine + 2;

		size_t colonPos = line.find(":");
        if (colonPos != std::string::npos)
        {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            // Trim whitespace
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));

            client.setHeader(key, value);

            if (key == "Content-Length")
                client.setContentLength(std::atoi(value.c_str()));
        }
	}
}
bool Client::getRequestParsed() const {
    return request_parsed;
}
void Client::setRequestParsed(bool parsed) {
    request_parsed = parsed;
}
void Client::receiveData(webserv &connection, int fd, ServerConfig &config)
{
    while (true)
    {
        char buffer[4096];
        int bytes = recv(fd, buffer, sizeof(buffer), 0);
        if (bytes == 0)
            break;
        if (bytes < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break; // No more data available for now
            else
            {
                close(fd);
                connection.fd_contexts.erase(fd);
                return;
            }
        }
        // accumulate TCP stream
    //we can read again until we get EAGAIN, which means we have read all the data for now

    // accumulate TCP stream
    this->setRecvBuffer(this->getRecvBuffer() + std::string(buffer, bytes)); //concatenate new data to existing buffer
    if (!this->getRequestParsed()) {
        size_t pos = this->getRecvBuffer().find("\r\n"); // "\r\n" indicates end of request line
        if (pos != std::string::npos) // these two lines are a gimmick to know whether or not we can proceed with request line
        {
            std::string request_line = this->getRecvBuffer().substr(0, pos);
            this->setRequestParsed(true);
			this->request_line.method = request_line.substr(0, request_line.find(" "));
			this->request_line.url = request_line.substr(request_line.find(" ") + 1, request_line.rfind(" ") - request_line.find(" ") - 1);
			this->request_line.version = request_line.substr(request_line.rfind(" ") + 1);
            this->set_location_index(match_location(config, this->request_line.url)); //set location index in client struct, so that we can use it later when building response and stuff, instead of having to match location again
            	//std::cout << "Parsed request line: " << this->request_line.method << " " << this->request_line.url << " " << this->request_line.version << std::endl;
			// remove request line from buffer
			this->setRecvBuffer(this->getRecvBuffer().substr(pos + 2));
		}
	}

    // check if headers finished
    if (!this->getHeadersParsed()) // if we haven't parsed headers yet, check for header completion
    {
        size_t pos = this->getRecvBuffer().find("\r\n\r\n"); // headers end with "\r\n\r\n"
        if (pos != std::string::npos) // same gimmick
        {
            this->setHeadersParsed(true);

            std::string headerPart = this->getRecvBuffer().substr(0, pos);
            parseHeaders(*this, headerPart);

            // remove headers from buffer
            this->setRecvBuffer(this->getRecvBuffer().substr(pos + 4));
        }
    }


	//how to store body data in client struct ? we can just keep appending to recv buffer until content length is reached, and then we can parse the body and build the response, but we need to make sure we don't do extra work in the main loop, so we can check for body completion here in receiveData
	//, and if body is complete, we can build the response and set POLLOUT for this client
    // if we know content length, check body completion
    if (this->getHeadersParsed() && this->getContentLength() > 0)
    {
        int max_body_size = config.locations[this->get_location_index()].body_size;
        if (this->getContentLength() > max_body_size)
        {
            // body too large, we can build 413 response immediately
            this->response = HttpResponse(Http::Status::PayloadTooLarge, "413 Request Entity Too Large");
            break;
        }
        if (static_cast<int>(this->getRecvBuffer().size()) >= this->getContentLength())
        {
			this->body = (this->getRecvBuffer().substr(0, this->getContentLength())); // set body to the received body data		
            this->setRecvBuffer(this->getRecvBuffer().substr(this->getContentLength())); // remove body from buffer, in case there is pipelining or extra data after the body, we want to keep it in the buffer so that we can process it as part of the next request without losing any data
            // now we want POLLOUT
            for (size_t i = 0; i < connection.fds.size(); i++)
                if (connection.fds[i].fd == fd)
                    connection.fds[i].events = POLLOUT; // manually set POLLOUT for this client
			this->set_client_state(READY_TO_PROCESS); // set client state to ready to process, so that in the main loop when we see POLLOUT, we know we can build response for this client
            break;
        }
    }
	else if (this->getHeadersParsed() && this->getContentLength() == 0)
	    {
		// no body, we can build response immediately
		//buildResponse(*this);

		// now we want POLLOUT
		    for (size_t i = 0; i < connection.fds.size(); i++)
			    if (connection.fds[i].fd == fd)
				    connection.fds[i].events = POLLOUT; // manually set POLLOUT for this client
		    this->body = ""; // set body to empty string for consistency, since we know there is no bodyµ
            //i am not sure about buffer content here for now might want to debug that
            this->set_client_state(READY_TO_PROCESS); // set client state to ready to process, so that in the main loop when we see POLLOUT, we know we can build response for this client
            break;
	    }
    }
}

void Client::reset() {
    setRecvBuffer("");
    clearHeaders();
    setBody("");
    set_location_index(-1);
    setRequestParsed(false);
    setHeadersParsed(false);
    setContentLength(0);
    set_client_state(READING);
}
ClientState Client::get_client_state() const {
    return this->state;
}
cgi_handles Client::get_cgi_handles() const {
    return this->cgi;
}
void Client::set_cgi_handles(pid_t pid, int input_fds[2], int output_fds[2], std::string cgi_input, std::vector<std::string> env) {
    this->cgi.pid = pid;
    this->cgi.input_fds[0] = input_fds[0];
    this->cgi.input_fds[1] = input_fds[1];
    this->cgi.output_fds[0] = output_fds[0];
    this->cgi.output_fds[1] = output_fds[1];
    this->cgi.env = env;
    this->cgi.input_data = cgi_input;
}
void Client::set_client_state(ClientState state) {
    this->state = state;
}
