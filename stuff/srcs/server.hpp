#ifndef SERVER_HPP
# define SERVER_HPP

# include "config_handler.hpp"
# include "client.hpp"
# include "request.hpp"
# include "response.hpp"
# include "webserv.hpp"

# define RESET "\033[0m"
# define GREEN "\033[32m"
# define RED "\033[31m"

class server
{
	public:
		server(void);
		~server(void);
		server(const server &other);
		server	&operator=(const server &other);

		void	config(std::string conf_file);
		int		setup(void);
		void	run(void);

	private:
		std::map<std::string, config_handler>	_config;
		int								_timeout;
		int								_total_clients;
		std::vector<int>				_server_fds;
		std::vector<int>				_requests_fd;
		std::vector<struct pollfd>		_pollfds;
		std::map<int, client>			_socket_clients;
		std::map<int, request *>		_fd_request_client; 

		void									_fileToserver(std::string &conf_file);
		int										_listenPoll(void);
		bool									_checkingRevents(void);
		bool									_pollin(std::vector<pollfd>::iterator it);
		bool									_pollout(std::vector<pollfd>::iterator it);
		int										_receiving(std::vector<pollfd>::iterator it, std::map<int, client>::iterator client);
		bool									_sending(std::vector<pollfd>::iterator it, std::map<int, client>::iterator client);
		bool									_acceptConnections(int server_fd);
		void									_closeConnection(std::vector<pollfd>::iterator it);
		std::string								_getRightconfig_handlerName(std::string &host);
		void									_verifyHost(std::string &host);
		void									_getHostInBuffer(std::string &buffer, std::string &host, std::string &uri);
		void									_setclientPollFd(std::vector<pollfd>::iterator it, int event);
		std::vector<std::vector<std::string> >	_getConfOfFile(std::string &conf);
};

#endif
