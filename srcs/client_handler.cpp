#include "client_handler.hpp"
#include "color.hpp"


client_handler::client_handler(struct_epoll &epoll, request_handler& rqst) : _epoll(epoll), _rqst(rqst)
{
#ifdef _log_
	ofstream _LOGfile("log.txt", std::ofstream::trunc);
	if (!_LOGfile.is_open())
		throw std::runtime_error("OPEN FAILLED (client_handler::client_handler)");
	_LOGfile.close();
#endif
}

client_handler::~client_handler()
{
}

// Fonction générique pour déterminer si un requête est fini
bool client_handler::is_request_fulfilled(int id)
{
#ifdef _debug_
	cout << BLUE "DANS IS_REQUEST_FULFILLED" RESET " size of current reqst : " << clients[_epoll._events[id].data.fd].rqst.length() <<"\n";
	if (clients[_epoll._events[id].data.fd].rqst.length() < 1000)
		cout << "Client N°" << _epoll._events[id].data.fd << " request : \n[" << clients[_epoll._events[id].data.fd].rqst + "]" << endl;
	else
		cout << "Client N°" << _epoll._events[id].data.fd << " request > 1000octets " MAGENTA " non affichée\n" RESET;	
#endif

	client_info& client = clients[_epoll._events[id].data.fd];
	if (client.request_fulfilled)
		return true;
	// else if (client.rqst_t == INVALID && !request_type(client)) // ON NE TRAITE QUE SI LE client.rqst_t N'EST PAS DÉFINI
	// 	return false;
	else if (!client.rqst_transfer_t && !request_transfer_type(client)) // ON NE TRAITE QUE SI LE client.rqst_transfer_t N'EST PAS DÉFINI
		return false;
	else if (!client.request_fulfilled && !is_fulfilled(client))
		return false;

#ifdef _debug_	
	cout << RED "return (client.request_fulfilled = true) ;\n" RESET;
#endif

	return (client.request_fulfilled = true) ;
}

// Détermine le type de transfer utilisé par le client ("multipart", "x-www-form-urlencoded", "chunked")
bool client_handler::request_transfer_type(client_info& client)
{
#ifdef _debug_
	cout << BLUE "DANS REQUEST_TRANSFER_TYPE()\n";
#endif

	if ( (client.header_end = client.rqst.find("\r\n\r\n")) == string::npos) {// Vérifie qu'on a au moins les headers
#ifdef _debug_
		cout << YELLOW "Header imcomplet\n" RESET;
#endif
		return (client.rqst_transfer_t = NONE);
	}

	size_t pos2;
	// if (client.rqst_t == HEAD)
	// 	return true;
	if ( (pos2 = portion_search(client.rqst, "Transfer-Encoding:", 0, client.header_end)) != string::npos && pos2 < client.header_end ) {
		cout << RED "Transfer-Encoding: 1ere ÉTAPE Client N°" << client.c_id << "\n" RESET;
#ifdef _debug_
#endif
		if ( (pos2 = portion_search(client.rqst, "chunked", pos2, client.header_end)) != string::npos && pos2 < client.header_end )
			return (client.rqst_transfer_t = CHUNCK);
	}
	else if ( (client.header_end += 4) && (pos2 = portion_search(client.rqst, "Content-Type:", 0, client.header_end)) != string::npos && pos2 < client.header_end ) {
		if ( (pos2 = portion_search(client.rqst, "Content-Length:", 0, client.header_end)) != string::npos && pos2 < client.header_end ) {
			pos2 += 15; // 15 == "Content-Length:"
			client.clen = strtol(&client.rqst[pos2], NULL, 10); // Stockage de Content-Length ds le client_info
		}
		if ( (pos2 = portion_search(client.rqst, "multipart/form-data", 0, client.header_end)) != string::npos && pos2 < client.header_end ) {
			// ON CHOPE LA BOUNDARY : boundary=[------------------------772ed66a82c8bebbM] (QUE C QU'IL Y A ENTRE LES CROCHETS)
			size_t limit_end  = client.rqst.find_first_of("\r\n", client.rqst.find("boundary=--------", pos2));
			size_t limit_beg  = client.rqst.find_last_of('=', limit_end) + 1;
			if (limit_end == string::npos || limit_beg == string::npos)
				return false;
			// Stockage de la boundary de la requete post ds la struct client_info (client.post_boundary)
			cout << RED "Post_boundary : " RESET << (client.post_boundary = client.rqst.substr(limit_beg, limit_end - limit_beg)) << endl;
			return (client.rqst_transfer_t = MULTIPART);
		}
		else if ( (pos2 = portion_search(client.rqst, "application/x-www-form-urlencoded", 0, client.header_end)) != string::npos && pos2 < client.header_end )
			return (client.rqst_transfer_t = URL_ENCODED);
	}
	else if (!(client.header_end - client.rqst.size())) { // S'il n'y a pas de body après les headers
#ifdef _debug_
		cout << RED "return (client.rqst_transfer_t = NO_BODY);\n" RESET;
#endif
		return (client.rqst_transfer_t = NO_BODY);
	}
#ifdef _debug_
	cout << RED "return (client.rqst_transfer_t = NONE);\n" RESET;
	if (client.rqst.size() <= 1000)
		cout << " reqst : \n"  << "[" + client.rqst + "]\n";
#endif
	return (client.rqst_transfer_t = NONE);
}

// check selon le type de transfert si la requete est complète
bool client_handler::is_fulfilled(client_info& client)
{//https://datatracker.ietf.org/doc/html/rfc7231#section-4.3.3
#ifdef _debug_
	cout << BLUE "DANS is_fulfilled, size of current reqst : " << client.rqst.length() << "\n" RESET;
#endif

	// if (client.rqst_t == HEAD)
	// 	return true;
	if (client.rqst_transfer_t == MULTIPART) {
	// SI ON EST A LA FIN ON DEVRAIT AVOIR LE DELIMITEUR AV "--" EN PREFIXE & SUFIXE : on prends la fin de la requete...
#ifdef _debug_
		cout << YELLOW "MULTIPART : \n" RESET;
#endif
		// cout << client.rqst << endl;
		if (client.rqst.substr(client.rqst.size() - 46).find("--" + client.post_boundary + "--") != string::npos
			&& client.rqst.size() - client.header_end >= client.clen )
			return true;
	}
	else if (client.rqst_transfer_t == URL_ENCODED) {
#ifdef _debug_
		cout << YELLOW "URL_ENCODED : " << client.rqst.size() - client.header_end << " client.clen : " << client.clen << "\n" RESET;
#endif
		if (client.rqst.size() - client.header_end >= client.clen)
			return true;
	}
	else if (client.rqst_transfer_t == CHUNCK) {	
	// SI ON EST A LA FIN ON DEVRAIT AVOIR LE "0\r\n\r\n" du chunck de fin : on prends la fin de la requete...
#ifdef _debug_
		cout << YELLOW "CHUNCK :\n" RESET;
#endif
		// cout << client.rqst << endl;
		if (client.rqst.find("0\r\n\r\n", client.rqst.size() - 5) != string::npos)
			return true;
	}
	else if (client.rqst_transfer_t == NO_BODY) {
#ifdef _debug_
		cout << YELLOW "NO_BODY :\n" RESET;
#endif
		if (client.rqst.find("\r\n\r\n", client.rqst.size() - 4) != string::npos)
			return true;
	}
#ifdef _debug_
	cout << RED "Not yet...\n" RESET;
#endif
	return false;
}

void client_handler::clear(int id) {	clients[_epoll._events[id].data.fd].rqst.clear(); 
	client_info& c = clients[_epoll._events[id].data.fd];
	c.rqst.clear();
	c.resp.clear();
	c.post_boundary.clear();
	c.post_file_path.clear();
	c.rqst_time_start = 0;
	c.time_out = 0;
	c.cgi_fd[0] = 0;
	c.cgi_fd[1] = 0;
	c.ext_id = 0;
	c.rqst_t = 0;
	c.rqst_transfer_t = 0;
	c.request_fulfilled = 0;
	c.header_end = 0;
	c.byte_send = 0;
	c.cgi_byte_write = 0;
	c.clen = 0;

	c.serv = NULL;
	c.loc = NULL;
}

void client_handler::rqst_append(int id, char *str, int byte_recved) { clients[_epoll._events[id].data.fd].rqst.append(str, byte_recved); }
client_info& client_handler::get_info(int id) { return clients[_epoll._events[id].data.fd]; }

// si la requete d'un des clients est plus longue a traiter que son time_out (set ds la config) on ferme la connexion ... puis on remove le client
void client_handler::check_all_timeout(void) 
{	
	for (std::map<int, client_info>::iterator it = clients.begin(); it != clients.end(); it++)
		if (time(NULL) - it->second.rqst_time_start > it->second.time_out) { // COPY DE REMOVE CERTAINEMENT MIEUX A FAIRE...
			cout << "elapsed time : " << time(NULL) - it->second.rqst_time_start <<  ", time_out du client N°" << it->first << " : " <<  it->second.time_out << endl;
			epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_DEL, it->first, &_epoll._event);
			if (close(it->first)) 
				throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
			cout << BLUE "DS (time_out)" RESET "close(" << it->first << ")\n";
			clients.erase(it->first);
		}
}

// Ajoute un client à l'intance e_poll, à la structure client_info et initialize son time_out
void client_handler::add(int time_out, int i)
{
	int client_fd;
	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);
	if ((client_fd = accept(_epoll._events[i].data.fd, (struct sockaddr *)&clientaddr, &len)) < 0)
		throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");

	cout << BLUE "DS (CLIENT) ADD() " RESET "accept client_fd(" << client_fd << ")\n";
#ifdef _debug_
#endif

	// clientaddr.sin_addr;
	_epoll._event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
	_epoll._event.data.fd = client_fd;
	if(epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_ADD, client_fd, &_epoll._event)) {
		fprintf(stderr, "Failed to add file descriptor to epoll\n");
		// close(_epoll_fd);
		throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}	
	clients[client_fd].time_out = time_out;
	time(&clients[client_fd].rqst_time_start);
	clients[client_fd].c_id = client_fd;
#ifdef _debug_
	cout << BLUE "DS CLIENT_ADD()" RESET " client_fd : " << client_fd
	 << " _epoll._events[i].data.fd; : " << _epoll._events[i].data.fd << endl;
#endif
}

void client_handler::remove(int i)
{
	cout << BLUE "DS (CLIENT) REMOVE() close(" << _epoll._events[i].data.fd << ")\n" RESET;
#ifdef _debug_
#endif
	epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_DEL, _epoll._events[i].data.fd, &_epoll._event);
	clients.erase(_epoll._events[i].data.fd);
	if (close(_epoll._events[i].data.fd)) 
		throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
}

// Envoie la reponse du client et efface le client si tout a été envoyé
void client_handler::send(int id)
{
	client_info& c = clients[_epoll._events[id].data.fd];
	// if (!c.request_fulfilled)
	// 	return ;

#ifdef _debug_
	cout << BLUE "DS SEND byte sent : " RESET << c.byte_send;
	cout << " client fd : " << c.c_id << endl;
	if (c.resp.length() < 1000)
		cout << "c.resp : \n[" << c.resp << "]\n";
#endif
	if (c.request_fulfilled /* && c.byte_send < c.resp.length() */) {
		int test;

		c.byte_send += test = ::send(_epoll._events[id].data.fd, &c.resp[c.byte_send], c.resp.length() - c.byte_send, 0);
	cout << GREEN "!! SEND !! c.byte_send : " RESET << c.byte_send << " / " << c.resp.length() << " - test : " << test << " to client N°" << _epoll._events[id].data.fd << " c.request_fulfilled ? " << (int)c.request_fulfilled << endl;
#ifdef _debug_
#endif
	// if (c.byte_send == c.resp.length()) {
	if (test <= 0) {

	cout <<  RED "!! DS SENT CLIENT REMOVED !! test : " RESET << test << endl;
	cout <<  GREEN << c.resp.substr(0, c.resp.find("\r\n\r\n")) << RESET << endl;
#ifdef _debug_
#endif
		// c.byte_send = 0;
		// remove(id);
		// clients.erase(_epoll._events[id].data.fd); // ça ça plante
		// clear(id);
	}
	}
}

//Return the position of needle found in haystack otherwise return npos
//Beware that haystack must be bigger than needle and its boundary accurate, otherwise 😕🤮
size_t client_handler::portion_search(string haystack, string needle, size_t from, size_t to)
{
	string::iterator first  = haystack.begin() + from,
					 last   = haystack.begin() + (to > haystack.size() ? haystack.size() : to),
					 s_last = needle.end();
	while (1) {
		string::iterator it = first;
		for (string::iterator s_it = needle.begin(); ; ++it, ++s_it) {
			if (s_it == s_last) return first - haystack.begin();
			if (it == last) return string::npos;
			if (!(*it == *s_it)) break;
		}
		++first;
	}
}
