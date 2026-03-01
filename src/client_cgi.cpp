#include "webserv.hpp"

void WriteToCgiInput(const std::string& input, int input_fd) {
	ssize_t nw;
	if (!input.empty()) {
		while ((nw = write(input_fd, input.c_str(), input.size())) < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// If the pipe is full, we can wait for it to be writable again
				fd_set write_fds;
				FD_ZERO(&write_fds);
				FD_SET(input_fd, &write_fds);
				select(input_fd + 1, NULL, &write_fds, NULL, NULL);
			} else {
				perror("write");
				return; // Handle error appropriately
			}
		}
		(void)nw; // silence unused-result warning
	}
}
// we could use throw instead of error codes whichever you prefer i have no qualms about either
int	Client::read_response_from_cgi(webserv &connection, int fd) {
	char buffer[4096];

	size_t max_body_size = connection.servers[connection.fd_contexts[fd].server_index].locations[this->get_location_index()].body_size;
	while (true) // read all available data from CGI output until EAGAIN
	{
	ssize_t n = read(fd, buffer, sizeof(buffer));

	if (n > 0)
	{
		this->response.setBody(std::string(buffer, n)); // accumulate CGI output into response body
		if (this->response.getContentLength() > max_body_size)
		{
			// CGI output too large, we can build 413 response immediately
			this->response = HttpResponse(Http::Status::PayloadTooLarge, "413 Request Entity Too Large");
			return -3; // return error code if CGI output exceeds max body size
		}
	}
	else if (n == 0) // EOF from CGI output means CGI script finished execution
	{
		close(fd); // close the CGI output fd
		for (size_t i = 0; i < connection.fds.size(); i++)
			if (connection.fds[i].fd == fd)
				connection.fds[i].events = POLLOUT; // set POLLOUT to send response back to client
		this->set_client_state(READY_TO_PROCESS); // set client state to ready to process, so
		// that in the main loop when we see POLLOUT, we know we can build response for this client
		int status = 0;
		int return_val = waitpid(connection.fd_contexts[fd].cgi_pid, &status, WNOHANG);
		if (return_val < 0) {
			std::cerr << "waitpid() failed: " << strerror(errno) << std::endl;
			return -2; // return error code if waitpid fails
		}
		break;
	}
	else if (n < 0 && errno != EAGAIN) // in case of error other than EAGAIN, we should close the connection
	{
		return -1; // return error code if read fails for reasons other than EAGAIN
	}
	else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		// No more data to read for now, we can wait for the next POLLIN event to read more CGI output
		break;
	}
	}
	return 0; // return success code if read was successful
}