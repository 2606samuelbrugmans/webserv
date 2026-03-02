#include "webserv.hpp"

void Client::sendData(webserv &connection, int fd)
{

    const std::string &packet = this->response.getSerialized();

    ssize_t n = send(fd,
                     packet.c_str() + bytes_sent,
                     packet.size() - bytes_sent,
                     0);

    if (n > 0)
        bytes_sent += n;
    else if (n < 0 && errno != EAGAIN) // in case of error other than EAGAIN, we should close the connection
    {
        delete this; // free client resources
        return;
    }

    // Finished sending entire HTTP message
    if (bytes_sent == packet.size())
    {
        std::cout << "Finished sending response to client fd " << fd << std::endl;
        reset();      
                   // clear parsing state you don't want to clear the buffer
        // actually cause it can have the next request, but you want to clear the headers and body and stuff
        bytes_sent = 0;

        // back to reading next request
        for (size_t i = 0; i < connection.fds.size(); i++)
            if (connection.fds[i].fd == fd)
                connection.fds[i].events = POLLIN;
        this->set_client_state(READING); // set client state back to reading for the next request
    }
}