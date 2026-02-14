#include "webserv.hpp"


int main(int argc, char* argv[]) {

	Connection connection;
	fileConfig fileConfig;
	fileConfig.parseConfigFile("config.conf"); // i could do fileconfig 
	//and its parser this weekend if you're focused on the rest
	connection.fileconfig = fileConfig;
	try {
		socketing(connection);
		polling(connection.servers[0].fd, connection);
		//there can be lots of servers, but for now we just have one
		//number of servers will be determined by config file
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	while (true)
	{
    	poll(&connection.fds[0], connection.fds.size(), -1);
    	for (size_t i = 0; i < connection.fds.size(); i++)
    	{
			pollfd p = connection.fds[i];
		if (p.revents & POLLIN)
			{
    		if (isListeningSocket(connection, p.fd))      // new client ?
        		acceptNewClient(connection, p.fd);
    		else                              // already accepted client
        		receiveData(connection, p.fd);
			}
			else if (p.revents & POLLOUT)
        	{
            	sendData(connection, p.fd);              // not even prototyped yet, but you get the idea
        	}
		}
	}
	/*
	while (true)
	{
    poll(all_descriptors);

    for each fd that woke up
        route event to correct object
	}
		*/
	return 0;
}