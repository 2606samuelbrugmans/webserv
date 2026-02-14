#include "webserv.hpp"

Client::Client(int fd) : client_fd(fd) {
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

void acceptNewClient(Connection &connection, int fd)
{
    while (true) // one or more clients may be waiting to connect
    {
        sockaddr_in addr;
        socklen_t len = sizeof(addr);

        int client_fd = accept(fd, (sockaddr*)&addr, &len);

        if (client_fd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) //clients to accept ?
                break; // no more clients to accept, exit loop
            return; // negative fd equal error if not EAGAIN or EWOULDBLOCK
        }

        fcntl(client_fd, F_SETFL, O_NONBLOCK); //avoid blocking on this client socket
		polling(client_fd, connection); // add client fd to polling list
        connection.clients.insert(std::make_pair(client_fd, Client(client_fd)));

        //std::cout << "New client with fd: " << client_fd << std::endl;
    }
}
void receiveData(Connection &connection, int fd)
{
    char buffer[4096];
    int bytes = recv(fd, buffer, sizeof(buffer), 0);

    if (bytes <= 0)
    {
        close(fd);
        connection.clients.erase(fd);
        return;
    }

    Client &client = connection.clients[fd];

    // accumulate TCP stream
    client.setRecvBuffer(client.getRecvBuffer() + std::string(buffer, bytes)); //concatenate new data to existing buffer

    // check if headers finished
    if (!client.getHeadersParsed()) // if we haven't parsed headers yet, check for header completion
    {
        size_t pos = client.getRecvBuffer().find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            client.setHeadersParsed(true);

            std::string headerPart = client.getRecvBuffer().substr(0, pos);
            parseHeaders(client, headerPart);

            // remove headers from buffer
            client.setRecvBuffer(client.getRecvBuffer().substr(pos + 4));
        }
    }

    // if we know content length, check body completion
    if (client.getHeadersParsed() && client.getContentLength() > 0)
    {
        if (client.getRecvBuffer().size() >= client.getContentLength())
        {
            client.setRecvBuffer(client.getRecvBuffer().substr(0, client.getContentLength())	);

            /// buildResponse(client);

            // now we want POLLOUT
            for (size_t i = 0; i < connection.fds.size(); i++)
                if (connection.fds[i].fd == fd)
                    connection.fds[i].events = POLLOUT; // manually set POLLOUT for this client
        }
    }
}


void removeClient(Connection &connection, int fd)
{
    close(fd);
    connection.clients.erase(fd);

    for (std::vector<pollfd>::iterator it = connection.fds.begin();
         it != connection.fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            connection.fds.erase(it);
            break;
        }
    }
}
