#include <map>
#include <string>
#include <netdb.h>		// pour struct sockaddr_in
#include <unistd.h>		// pour close
#include <iostream>
#include <sys/epoll.h>
#include "struct_webserv.hpp"

#include <sstream>
#include <vector>
#include <cstdlib>
#include <fcntl.h>
#include <cstring>

#include <string>

#define MAX_LEN 8192

using namespace std;

class client_handler
{
private:
	map <int, client_info> clients;

public:
	client_handler(/* args */);
	~client_handler();

	bool is_request_fulfilled(int);
	void remove(struct_epoll& _epoll, int i); // REMOVE A CLIENT
	void remove_fd(struct_epoll& _epoll, int fd); // REMOVE A CLIENT
	void add(struct_epoll& _epoll, int time_out, int i); // ADD A CLIENT
	void check_all_timeout(struct_epoll& _epoll);

	/* Function rubrique : getter setter */
	void clear(int client_fd); // CLEAR CLIENT CONTENT
	void rqst_append(int , char *, size_t); // APPEND DIFFERENT REQUEST CHUNK
	string get_rqst(int); // RETURN FINAL REQUEST

	/* FUNCTION SECONDAIRE : UTILITAIRES */

	client_info&	get_info(int fd) { return (clients[fd]); };
	bool is_post_rqst_fulfilled(client_info& client);
	bool is_chunked_rqst_fulfilled(client_info& client);
	std::vector<int>	handle_chunks(struct_epoll& _epoll);
	void	fill_resp(int fd, std::string& base);
	int		chunked_rqst(struct_epoll& _epoll, int fd);
	int		chunked_resp(struct_epoll& _epoll, int fd);
	void 	time_reset(struct_epoll& _epoll, int time_out, int fd); // Reset client fd
	void 	rearm(struct_epoll& _epoll, int time_out, int fd); // Reset client fd
	int		no_chunk(int fd);
};
