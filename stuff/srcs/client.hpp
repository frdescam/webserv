#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"
# include "request.hpp"
# include "response.hpp"
# include "config_handler.hpp"

class client
{
	public:
		client(void);
		~client(void);
		client(client const &other);
		client(struct pollfd fd);
		client	&operator=(client const &other);

		response		&getresponse(void);
		void			addTorequest(std::string &str, int rc, config_handler &block);
		void			addToresponseLength(size_t block_size);
		void			setId(int new_id);
		int				getrequestFd(void);
		struct pollfd	getrequestPollFd(void);
		struct pollfd	&getclientPollFd(void);
		request			*getrequestPtr(void);
		void			resetrequest(void);

	private:
		struct pollfd	_client_fd;
		request			*_http_request;
		response		_http_response;
		int				_id;
		struct pollfd	_request_poll_fd;
		int				_request_fd;
};

#endif
