#ifndef _STRUCT_WEBSERV_HPP_
#define _STRUCT_WEBSERV_HPP_
#include <sys/epoll.h>	// for epoll_create1(), epoll_ctl(), struct epoll_event 
#include <string>
#include <iostream>
#include <vector>
#define MAX_EVENTS 1000

// POST_MULTIPART = rqst post regulière (avec boundary=), POST_CHUNCK = rqst post chunked (no boundary=)
enum e_rqst_type		{GET, PUT, POST, HEAD, DELETE, INVALID};
enum e_rqst_tranfer_type{NONE, MULTIPART, CHUNCK, URL_ENCODED, NO_BODY};

struct locati_info {
	std::string	location;							// directive location rattachée à l'url demandée
	std::string download_path;						// path de stockage des fichiers upload sur le server
	std::string auth_user_file;						// fichiers autorisés à l'utilisateur
	std::string autoindex;							// présentation du fileSystem => val (on/off)
	std::string index;								// fichier servi par défault
	std::string max_file_size;						// Sets the maximum allowed size of the client request body. If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client. Please be aware that browsers cannot correctly display this error. Setting size to 0 disables checking of client request body size. 
	std::string return_directive;					// redirection vers une autre location
	std::string root;								// dossier racine
	std::string cgi_path;							// dossier où est l'executable de traitement des cgi
	std::vector<std::string> cgi_file_types;		// type de file gérées pour les cgi
	std::vector<std::string> allowed_method;		// méthodes (GET, POST, etc) permises
	std::vector<std::string> retour;				// directive return
};

struct server_info {
	int socket;
	std::string port;
	std::string host;
	std::string server_name;
	std::string error_page;							// Path des pages d'erreurs renseigné ds la config (si pas renseigné il y a des pages d'erreurs par default)
	std::string max_file_size;						// Sets the maximum allowed size of the client request body. If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client. Please be aware that browsers cannot correctly display this error. Setting size to 0 disables checking of client request body size. 
	std::string time_out;							// Temps imparti à la gestion d'une requete avant d'être rejetée par un time_out
	std::string cgi_path;							// Path de l'executable chargé des cgi
	std::vector<locati_info> location;
};

struct client_info {
	std::string rqst;								// Requete envoyée par le client
	std::string	resp;								// Réponse à renvoyer au client
	std::string post_boundary;						// boundary=([)------------------------f3a140510ee62d32(])
	std::string	post_file_path;						// Chemin du fichier demandé par les requetes (POST & PUT)
	time_t		rqst_time_start;					// Moment à partir duquel on mesure le time_out d'une requete
	int			time_out;							// Temps de time_out renseigné ds la config
	int			cgi_fd[2];							// Fd servant aux IO des cgi
	char		ext_id;								// [Id - 1] de l'extension du fichier à gérer pour les cgi contenu ds -> _si[_s_id].location[_l_id].cgi_file_types[_c->ext_id]
	char		rqst_t;								// type de requete (GET, PUT, POST, HEAD, DELETE, INVALID)
	char		rqst_transfer_t;					// type de transfert de la requete (Chunk, multipart, etc.)
	char		request_fulfilled;					// Etape de traitement de la requete
	size_t		header_end;							// position de la fin des headers
	size_t		byte_send;							// Total byte sent over the body_length
	size_t		cgi_byte_write;						// Total byte write over the body_length
	size_t		clen;								// Len du body (pas de la rqst)

	server_info* serv;								// Pas utilisé pour le moment
	locati_info* loc;								// Pas utilisé pour le moment
	int			c_id;								// ne sert qu'à du debug (savoir l'id du client on avait un doute que epoll et le client socket soit different) -> pour les broken pipe
};

struct struct_epoll
{
	int _epoll_fd;
	int _event_count;
	epoll_event _event;
	epoll_event _events[MAX_EVENTS];
};

/* 
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

// trim from start
static inline std::string &ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(),
						std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(),
						std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
		return ltrim(rtrim(s));
}
 */

#endif