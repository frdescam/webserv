#include <map>
#include <string>
#include <netdb.h>		// pour struct sockaddr_in
#include <unistd.h>		// pour close
#include <iostream>
#include <fstream>
#include <sys/epoll.h>
#include "struct_webserv.hpp"
#include "request_handler.hpp"

using namespace std;

class client_handler
{
private:
	struct_epoll &_epoll; // ref sur la structure _epoll contenu par le server
	request_handler& _rqst;
	map <int, client_info> clients;

public:
	client_handler(struct_epoll &, request_handler &);
	~client_handler();
	bool is_request_fulfilled(int);
	bool request_transfer_type(client_info& client);
	bool is_fulfilled(client_info& client);
	void add(int time_out, int i); // ADD A CLIENT
	void remove(int i); // REMOVE A CLIENT
	void check_all_timeout(void);
	void send(int id);

	/* FUNCTION RUBRIQUE : GETTER SETTER */
	void clear(int id); // CLEAR CLIENT CONTENT
	void rqst_append(int id, char *str, int byte_recved); // APPEND DIFFERENT REQUEST CHUNK
	client_info& get_info(int); // RETURN FINAL REQUEST

	/* FUNCTION RUBRIQUE : UTILITAIRE */

	// recherche uniquement dans une sous-portion de la string
	size_t portion_search(string haystack, string needle, size_t from = 0, size_t to = string::npos);

};


