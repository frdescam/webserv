#include "client.hpp"

client::client(void) {}

client::~client(void)
{
	if (this->_http_request != NULL)
	{
		delete this->_http_request;
		this->_http_request = NULL;
	}
}

client::client(client const &other)
{
	*this = other;
}

client::client(pollfd fd)
{
	this->_client_fd = fd;
	this->_request_fd = -1;
	this->_http_request = 0;
}

client	&client::operator=(client const &other)
{
	if (this != &other)
	{
		this->_client_fd = other._client_fd;
		this->_http_request = other._http_request;
		this->_http_response = other._http_response;
		this->_id = other._id;
		this->_request_poll_fd = other._request_poll_fd;
		this->_request_fd = other._request_fd;
	}
	return (*this);
}

void	client::addTorequest(std::string &str, int rc, config_handler &block)
{
	if (this->_http_request != NULL)
	{
		if (this->_http_request->isChunked())
			this->_http_request->addToBodyChunked(str, rc);
		else
			this->_http_request->addToBody(str, 0, rc);
	}
	else
		this->_http_request = new request(str, rc, block, this->_id);
	this->_request_fd = this->_http_request->getFd();
	this->_request_poll_fd.fd = this->_request_fd;
	this->_request_poll_fd.events = 0;
}

void	client::addToresponseLength(size_t block_size)
{
	this->_http_response.addToLengthSent(block_size);
}

request			*client::getrequestPtr(void) {return this->_http_request;}
void			client::setId(int new_id) {this->_id = new_id;}
int				client::getrequestFd(void) {return this->_request_fd;}
struct pollfd	client::getrequestPollFd(void) {return this->_request_poll_fd;}
struct pollfd	&client::getclientPollFd(void) {return this->_client_fd;}
response		&client::getresponse(void) {return this->_http_response;}
