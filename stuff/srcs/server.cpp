#include "server.hpp"

extern bool g_end;

server::server(void): _config(), _timeout(20 * 60 * 1000), _total_clients(0)
{}

server::~server(void)
{
	for (size_t i = 0; i < this->_pollfds.size(); i++)
		close(this->_pollfds[i].fd);
	this->_pollfds.clear();
	std::cout << RED << "Cleaning and exiting..." << RESET << std::endl;
}

server::server(const server & other): _config(other._config), _timeout(other._timeout), _total_clients(other._total_clients)
{}

server &server::operator=(const server &other)
{
	if (this != &other)
	{
		this->_config = other._config;
		this->_timeout = other._timeout;
		this->_pollfds = other._pollfds;
		this->_socket_clients= other._socket_clients;
		this->_total_clients = other._total_clients;
		this->_requests_fd = other._requests_fd;
	}
	return (*this);
}

void server::_verifyHost(std::string &host)
{
	if (host.find("localhost") != std::string::npos)
		host.replace(0, 9, "127.0.0.1");
	if (host.find("0.0.0.0") != std::string::npos)
		host.replace(0, 7, "127.0.0.1");
}

void	server::config(std::string conf_file)
{
	if (conf_file.size() <= 5 || conf_file.compare(conf_file.size() - 5, 5, ".conf") != 0)
		throw std::runtime_error("Error: Wrong file type\n");
	if (pathIsFile(conf_file) != 1)
		throw std::runtime_error("Error: File doens't exist or is incorrect\n");
	this->_fileToserver(conf_file);
}

int	server::setup(void)
{
	struct pollfd		listening_fd;
	sockaddr_in			sock_structs;
	int					server_fd, yes = 1;

	this->_pollfds.reserve(CONNECTION_QUEUE);
	for(std::map<std::string, config_handler>::iterator it = this->_config.begin(); it != this->_config.end(); it++)
	{
		size_t	pos = it->first.find("/");
		if (it->first[pos + 1] != '1')
			continue;
		server_fd = -1;
		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			throw std::runtime_error("Error: socket() error\n");
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (int *) &yes, sizeof(yes)) == -1)
			throw std::runtime_error("Error: setsockopt() error\n");
		if (ioctl(server_fd, FIONBIO, &yes) < 0)
			throw std::runtime_error("Error: ioctl() error\n");
		sock_structs.sin_family = AF_INET;
		sock_structs.sin_port = htons(it->second.getPort());
		sock_structs.sin_addr.s_addr = inet_addr(it->second.getIpAddress().c_str());
		if (bind(server_fd, (sockaddr *)&sock_structs, sizeof(sockaddr_in)) < 0)
			throw std::runtime_error("Error: bind() error: " + it->first.substr(0, pos) + "\n");
		if (listen(server_fd, CONNECTION_QUEUE) < 0)
			throw std::runtime_error("Error: listen() error\n");
		this->_server_fds.push_back(server_fd);
		listening_fd.fd = server_fd;
		listening_fd.events = POLLIN;
		this->_pollfds.push_back(listening_fd);
		std::cout << "Listening on: " << it->first.substr(0, pos) << std::endl;
	}
	std::cout << GREEN << "Start..." << RESET << std::endl;
	return (0);
}

void	server::_closeConnection(std::vector<pollfd>::iterator it)
{
	close(it->fd);
	if (this->_socket_clients.find(it->fd) != this->_socket_clients.end())
	{
		if (this->_socket_clients.find(it->fd)->second.getrequestPtr() != NULL)
		{
			if (this->_socket_clients.find(it->fd)->second.getrequestPtr()->getFp() != NULL)
				fclose(this->_socket_clients.find(it->fd)->second.getrequestPtr()->getFp());
		}
		this->_socket_clients.erase(it->fd);
	}
	this->_pollfds.erase(it);
}

bool	server::_acceptConnections(int server_fd)
{
	struct pollfd	client_fd;
	int				new_socket = -1;

	do
	{
		new_socket = accept(server_fd, NULL, NULL);
		if (new_socket < 0) {
			if (errno != EWOULDBLOCK) {
				perror("accept() failed");
				return (true);
			}
			break ;
		}
		client_fd.fd = new_socket;
		client_fd.events = POLLIN;
		this->_pollfds.insert(this->_pollfds.begin(), client_fd);
		client new_client(client_fd);
		new_client.setId(this->_total_clients++);
		this->_socket_clients.insert(std::pair<int, client>(client_fd.fd, new_client));
	}
	while (new_socket != -1);
	return (false);
}

bool	server::_sending(std::vector<pollfd>::iterator	it, std::map<int, client>::iterator client)
{
	int 		i = 0;
	size_t 		block_size = BUFFER_SIZE;
	std::string response_block;

	if (BUFFER_SIZE > client->second.getresponse().getRemainingLength())
		block_size = client->second.getresponse().getRemainingLength();
	response_block = client->second.getresponse().getRawresponse().substr(client->second.getresponse().getLengthSent(), block_size);
	i = send(it->fd, response_block.c_str(), block_size, MSG_NOSIGNAL);
	if (i <= 0)
	{
		this->_closeConnection(it);
		return (1);
	}
	client->second.addToresponseLength(block_size);
	return (0);
}

int	server::_receiving(std::vector<pollfd>::iterator it, std::map<int, client>::iterator client)
{
	int 			rc = -1;
	std::string		buf(BUFFER_SIZE + 1, '\0');

	rc = recv(it->fd, (void *)buf.c_str(), BUFFER_SIZE, 0);
	if (rc <= 0)
	{
		this->_closeConnection(it);
		return (1);
	}
	if (client->second.getrequestPtr() != NULL) // REQUEST ALREADY EXISTS
	{
		if (!client->second.getrequestPtr()->getFlag())
			client->second.addTorequest(buf, rc, client->second.getrequestPtr()->getConf());
	}
	else // NEW REQUEST
	{
		std::string	host;
		std::string	uri;

		this->_getHostInBuffer(buf, host, uri);
		host.append(uri);
		this->_verifyHost(host);
		std::string configName = this->_getRightconfig_handlerName(host);
		if (configName.empty())
		{
			this->_closeConnection(it);
			return (1);
		}
		client->second.addTorequest(buf, rc, _config.at(configName));
		struct pollfd	client_request_pollfd = client->second.getrequestPollFd();
		if (client_request_pollfd.fd != -1) // IF POST
		{
			client_request_pollfd.events = POLLOUT;
			this->_pollfds.push_back(client_request_pollfd);
			this->_requests_fd.push_back(client_request_pollfd.fd);
			this->_fd_request_client.insert(std::pair<int, request *>(client_request_pollfd.fd, client->second.getrequestPtr()));
			return (2);
		}
	}
	return (0);
}

bool	server::_pollin(std::vector<pollfd>::iterator it) // Reading socket 
{
	std::map<int, client>::iterator client;
	std::vector<int>::iterator 		find;
	int								ret;

	find = std::find(this->_server_fds.begin(), this->_server_fds.end(), it->fd);
	if (find != this->_server_fds.end()) // socket of server 
	{
		g_end = this->_acceptConnections(*find);
		return (1);
	}
	else // Socket of client 
	{
		client = this->_socket_clients.find(it->fd);
		if (client != this->_socket_clients.end())
		{
			ret = this->_receiving(it, client);
			if (ret == 1)
				return (1);
			request *ptr = client->second.getrequestPtr();
			if (ptr != NULL && (ptr->isComplete() || (ptr->isChunked() && !ptr->sentContinue())))
				it->events = POLLOUT;
			if (ret == 2)
				return (1);
		}
	}
	return (0);
}

void	server::_setclientPollFd(std::vector<pollfd>::iterator it, int event)
{
	int	client_fd = -1;

	for (std::map<int, client>::iterator itb = this->_socket_clients.begin(); itb != this->_socket_clients.end(); itb++)
	{
		if (itb->second.getrequestPollFd().fd == it->fd)
		{
			client_fd = itb->second.getclientPollFd().fd;
			for (std::vector<pollfd>::iterator itpb = this->_pollfds.begin(); itpb != this->_pollfds.end(); itpb++)
			{
				if (itpb->fd == client_fd)
				{
					if (event == 1)
						itpb->events = POLLOUT;
					else if (event == 0)
					{
						this->_pollfds.erase(it);
						this->_closeConnection(itpb);
					}
					return ;
				}
			}
		}
	}
}

bool	server::_pollout(std::vector<pollfd>::iterator it) // Write socket
{
	std::map<int, client>::iterator		client;
	std::map<int, request *>::iterator	Request;
	std::vector<int>::iterator			find;
	int									ret = 0;

	client = this->_socket_clients.find(it->fd);
	if (client != this->_socket_clients.end()) // Socket of client
	{
		request	*client_request = client->second.getrequestPtr();
		if (client->second.getresponse().getRemainingLength() == 0) // Nothing sent yet 
		{
			if (client_request->isChunked() && !client_request->sentContinue())
				client->second.getresponse() = client_request->executeChunked();
			else
				client->second.getresponse() = client_request->execute();
		}
		if (this->_sending(it, client)) // Part of response sent 
			return (1);
		if (client->second.getresponse().isEverythingSent()) // All sent 
		{
			it->events = POLLIN; // Now reading the socket
			client->second.getresponse().reset();
			if (client_request->getFlag() == 413)
				return (1);
			if (client_request->isComplete()) // Reset if not chunk (cos sending 100-Continue)
			{
				find = std::find(this->_requests_fd.begin(), this->_requests_fd.end(), client->second.getrequestFd());
				if (find != this->_requests_fd.end())
				{
					this->_fd_request_client.erase(client->second.getrequestFd());  // Erasing of element request in map & vector (fd)
					this->_requests_fd.erase(find);
				}
			}
		}
		return (0);
	}
	find = std::find(this->_requests_fd.begin(), this->_requests_fd.end(), it->fd);
	if (find != this->_requests_fd.end()) // Fd (request)
	{
		Request = this->_fd_request_client.find(it->fd);
		ret = Request->second->writeInFile();
		if (ret <= 0)
		{
			_setclientPollFd(it, 0);
			return (1);
		}
		else if (Request->second->isComplete())
		{
			if (Request->second->getFp() != NULL)
			{
				fclose(Request->second->getFp());
				Request->second->setFpToNull();
			}
			_setclientPollFd(it, 1);
			this->_pollfds.erase(it);
			return (1);
		}
	}
	return (0);
}

bool	server::_checkingRevents(void)
{
	for (std::vector<pollfd>::iterator	it = this->_pollfds.begin(); it != this->_pollfds.end(); it++)
	{
		if (it->revents == 0)
			continue;
		else if (it->revents & POLLIN)
		{
			if (this->_pollin(it))
				break ;
		}
		else if (it->revents & POLLOUT)
		{
			if (this->_pollout(it))
				break ;
		}
		else if (it->revents & POLLERR) 
			this->_closeConnection(it);
	}
	return (g_end);
}

int		server::_listenPoll(void)
{
	int				rc = 0;
	unsigned int 	size_vec = (unsigned int)this->_pollfds.size();

	rc = poll(&this->_pollfds[0], size_vec, this->_timeout);
	if (rc <= 0)
		return (1);
	return (0);
}

void	server::run(void)
{
	while (!g_end)
	{
		if (this->_listenPoll())
		{
			g_end = true;
			return ;
		}
		g_end = this->_checkingRevents();
	}
}

void server::_getHostInBuffer(std::string &buffer, std::string &host, std::string &uri)
{
	std::vector<std::string> buff = mySplit(buffer, " \n\t\r");
	for (std::vector<std::string>::iterator it = buff.begin(); it != buff.end(); it++)
	{
		if (it->compare("GET") == 0)
			uri = *(it + 1);
		if (it->compare("Host:") == 0)
			host = *(it + 1);
	}
}

std::string server::_getRightconfig_handlerName(std::string &host)
{
	std::string	ret;
	size_t		found = 0;
	std::string	ip;
	std::string	uri;
	size_t		pos;

	pos = host.find_first_of("/");
	ip = host.substr(0, pos);
	if (pos != std::string::npos)
		uri = host.substr(host.find_first_of("/"), host.npos);
	for(std::map<std::string, config_handler>::iterator it = this->_config.begin(); it != this->_config.end(); it++)
	{
		if (it->first.find(ip) != std::string::npos)
			found++;
	}
	if (found == 1) // return the only one matching server{}
		return (ip + "/1");
	if (found > 1)
	{
		while (found) // checks locations on each matching server{}
		{
			std::stringstream	out;
			config_handler				tmp;

			out << found;
			tmp = this->_config.at(ip + "/" + out.str());
			for (std::map<std::string, config_handler>::iterator it = tmp.getLocation().begin(); it != tmp.getLocation().end(); it++)
			{
				if (it->first.compare(uri) == 0)
					return (ip + "/" + out.str());
			}
			found--;
		}
	}
	else // checks server_name on each maching server{} with port
	{
		std::string port = ip.substr(ip.find(":") + 1, ip.find("/") - ip.find(":"));
		ip = ip.substr(0, ip.find(":"));
		for(std::map<std::string, config_handler>::iterator it = this->_config.begin(); it != this->_config.end(); it++)
		{
			if (it->first.find(port) != it->first.npos)
			{
				for (size_t i = 0; i < it->second.getserverNames().size(); i++)
				{
					if (it->second.getserverNames()[i].compare(ip) == 0)
						return it->first;
				}
			}
		}
		return "";
	}
	return ip + "/1";
}

std::vector<std::vector<std::string> >	server::_getConfOfFile(std::string &conf) 
{
	std::ifstream							file(conf.c_str());
	std::string								line;
	std::vector<std::vector<std::string> >	confFile;
	std::vector<std::string>				tmp;

	if (file.is_open())
	{
		while (getline(file, line))
		{
			tmp = mySplit(line, " \n\t");
			if (!tmp.empty())
				confFile.push_back(tmp);
			tmp.clear();
		}
	}
	else
		throw std::runtime_error("Error: Cannot open configuration file\n");
	return confFile;
}

void	server::_fileToserver(std::string &conf_file)
{
	std::vector<std::vector<std::string> >	confFile;

	confFile = this->_getConfOfFile(conf_file);
	for (size_t i = 0; i < confFile.size(); i++)
	{
		if (confFile[i][0].compare("server") == 0 && confFile[i][1].compare("{") == 0)
		{
			std::stringstream	out;
			std::stringstream	countout;
			config_handler				block;
			int					count = 1;

			i = block.parseserver(confFile, i);
			block.checkBlock(false);
			out << block.getPort();
			for (std::map<std::string, config_handler>::iterator it = this->_config.begin(); it != this->_config.end(); it++) {
				if (it->first.find(block.getIpAddress() + ":" + out.str()) != std::string::npos)
					count++;
			}
			countout << count;
			this->_config.insert(std::pair<std::string, config_handler>(block.getIpAddress() + ":" + out.str() + "/" + countout.str(), block));
		}
		else if (confFile[i][0].compare("#") != 0)
			throw std::runtime_error("Error: Bad server{} configuration\n");
	}
}
