#include "server.hpp"
#include "config_checker.hpp"
#include "request_handler.hpp"
#include "client_handler.hpp"
#include <stdexcept>

#include <unistd.h> // Pour close, À ENLEVER

using namespace std;
bool _run = true;      // Initialisé à true tant qu'il n'y a pas de SIGINT

server::server(std::string av)
{
	config_checker confCheck;
	struct stat sb;
	if (stat(av.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
		confCheck.check_conFile(av);
	else 
		throw std::runtime_error("Bad configuration file given");
	this->_s = *confCheck._si; // je voulais le passer par reference mais pas possible av l'architecture actuelle
}

server::~server()
{
	cout << RED "server destructeur..." RESET << endl;
}

// PROBLEM POUR SIGNAL HANDLER PAS ENCORE FAIT PMARCHER
void	signal_handler(int signal)
{
	if (signal == SIGINT)
	{
		std::cout << CYAN << " Stopping server..." << RESET << std::endl;
		_run = false;
		// exit(0);
	}
}

void server::initialize(void) {
	struct sockaddr_in servaddr;

	signal(SIGINT, signal_handler);
	if ((_epoll._epoll_fd = epoll_create(1)) == -1) /* INITIALISATION DE L'INSTANCE EPOLL */
		throw std::runtime_error("ERROR IN EPOLL INSTANCE CREATION");
/* INITIALISATION DES SOCKET DES SERVER(S) VIRTUEL(S) */
	for (size_t i = 0; i < _s.size(); i++) {
		bool skip = false;
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET; 
		servaddr.sin_addr.s_addr = htonl(0); //127.0.0.1 -> 2130706433
		servaddr.sin_port = htons(atoi(_s[i].port.c_str()));
/* VERIFIE QUE LE SERVER A BIND N'AIT PAS D'HOMOLOGUE */
		for (int j = i - 1; i && j >= 0; --j)
				if (_s[j].host == _s[i].host && _s[j].port == _s[i].port && (skip = true))
					break ;
		if (skip)
			continue ;
/* CREATION DU SERVER */
		int	opt = 1;
		if ((_s[i].socket = socket(AF_INET, SOCK_STREAM, 0)) < 0
				|| setsockopt(_s[i].socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int))
				|| bind(_s[i].socket, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 
				|| listen(_s[i].socket, 0) < 0 )
			throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
/* && AJOUT DE CES DERNIERS À L'INSTANCE EPOLL */
		_epoll._event.events = EPOLLIN;
		_epoll._event.data.fd = _s[i].socket;
		if(epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_ADD, _s[i].socket, &_epoll._event))
			throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}
}

void server::run(void) {
	initialize();
	request_handler rqst(_s);
	client_handler client(_epoll, rqst);
	int byte_recved;
	int serv_id;

	while(_run)
	{
		_epoll._event_count = epoll_wait(_epoll._epoll_fd, _epoll._events, MAX_EVENTS, 10000); //500
#ifdef _debug_
		printf("\nPolling for input...\n");
		printf("%d ready events\n", _epoll._event_count);
#endif
		for(int i = 0; i < _epoll._event_count; ++i) {
			if ((serv_id = is_new_client(i)) >= 0 && (_epoll._events[i].events & EPOLLIN) == EPOLLIN) {
				client.add(get_time_out(serv_id), i);
#ifdef _debug_
				printf("New client added\n");
#endif
			}
			else if ( (_epoll._events[i].events & EPOLLRDHUP) == EPOLLRDHUP ) {
				printf(RED "EPOLLRDHUP " RESET "client N°%d REMOVED !!\n", _epoll._events[i].data.fd);
				client.remove(i);
				// sleep(1);
			}
			else if ( (_epoll._events[i].events & EPOLLIN) == EPOLLIN ) {
				bzero(str, sizeof(str)); // ON EFFACE UN HYPOTHÉTIQUE PRÉCÉDENT MSG
				byte_recved = recv(_epoll._events[i].data.fd, str, sizeof(str), 0);
				printf(RED "EPOLLIN " RESET "client N°%d byte_recved : %d\n", _epoll._events[i].data.fd, byte_recved);
#ifdef _debug_
				// sleep(1);
#endif
				if (byte_recved <= 0) {
						client.remove(i);
						// sleep(1);
						// break; // Pk Break T-ON ?
				}
				else { /* RECEPTION... ET TRAITEMENT DE LA REQUETE */
					// printf("client(fd : %d) msg : " YELLOW "\n%s\n" RESET,_events[i].data.fd,  str); 
					client.rqst_append(i, str, byte_recved);
					if (client.is_request_fulfilled(i)) {
#ifdef _debug_
						cout << "request_fulfilled !!\n";
#endif
						rqst.reader(client.get_info(i));
						rqst.writer();
						// client.send(i);
					}
				}
			}
			else if ( (_epoll._events[i].events & EPOLLOUT) == EPOLLOUT ) {
				printf(RED "EPOLLOUT " RESET "client N°%d\n", _epoll._events[i].data.fd);
#ifdef _debug_
				// sleep(1);
#endif

				// shutdown(_epoll._events[i].data.fd, SHUT_RD);
				client.send(i); // send() FERME LA CONNEXION ET VIRER LE CLIENT MS JE SAIS PAS SI ÇA DOIT ETRE FAIT COMMME ÇA
			}
		}
		client.check_all_timeout();
#ifdef _debug_
		// printf(RED "_event_count : %d\n" RESET, _epoll._event_count);
#endif
	}
	if(close(_epoll._epoll_fd))
	{
		fprintf(stderr, "Failed to close epoll file descriptor\n");
		throw std::runtime_error("ERROR IN FD (CLOSE) MANIPULATION");
	}
	return;
}

size_t server::is_new_client(int id) {
	for (size_t i = 0; i < _s.size(); ++i)
		if (_epoll._events[id].data.fd == _s[i].socket)
			return i;
	return -1;
}

int server::get_time_out(int id_serv) {
	return atoi(_s[id_serv].time_out.c_str());
};

/* 
* AFFICHE TOUTES LES INFORMATIONS CONTENUES DS LES STRUCTURES GÉNÉRÉES PAR LE FICHIER DE .CONF
*/
void server::display_server(void)
{
	for (size_t i = 0; i < _s.size(); i++)
	{
		cout << GREEN ITALIC UNDERLINE "DISPLAY SERVER INFORMATION" RESET GREEN " :" RESET << endl;
		// for (size_t j = 0; j < _s[i].server_name.size(); j++) // Qd server_name etait un vector
		cout << "server_name : " << _s[i].server_name << endl;
		cout << "time_out : " << _s[i].time_out << endl;
		cout << "port : " << _s[i].port << endl;
		cout << "host : " << _s[i].host << endl;
		cout << "error_page : " << _s[i].error_page << endl;
		cout << "max_file_size : " << _s[i].max_file_size << endl;
		cout << "cgi_path : " << _s[i].cgi_path << endl;
		for (size_t j = 0; j < _s[i].location.size(); j++) {
			cout << GREEN "LOCATION : " RESET << endl;
			cout << "location : " << _s[i].location[j].location << endl;
			cout << "download_path : " << _s[i].location[j].download_path << endl;
			cout << "auth_user_file : " << _s[i].location[j].auth_user_file << endl;
			cout << "autoindex : " << _s[i].location[j].autoindex << endl;
			cout << "index : " << _s[i].location[j].index << endl;
			cout << "max_file_size : " << _s[i].location[j].max_file_size << endl;
			cout << "return_directive : " << _s[i].location[j].return_directive << endl;
			cout << "root : " << _s[i].location[j].root << endl;
			cout << "cgi_path : " << _s[i].location[j].cgi_path << endl;
			cout << "cgi_file_types : ";
			for (size_t k = 0; k < _s[i].location[j].cgi_file_types.size(); k++)
				cout << _s[i].location[j].cgi_file_types[k] << (_s[i].location[j].cgi_file_types.size() - k + 1 ? ", " : "");
			cout << endl << "allowed_method : ";
			for (size_t k = 0; k < _s[i].location[j].allowed_method.size(); k++)
				cout << _s[i].location[j].allowed_method[k] << (k < _s[i].location[j].allowed_method.size() - k + 1 ? ", " : "");
			cout << endl << "return : ";
			for (size_t k = 0; k < _s[i].location[j].retour.size(); k++)
				cout << _s[i].location[j].retour[k] << (k < _s[i].location[j].retour.size() - k + 1 ? ", " : "");
			cout << endl;
		}
	}
}
