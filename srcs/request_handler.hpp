#ifndef _REQUEST_HANDLER_HPP_
#define _REQUEST_HANDLER_HPP_
#include <limits.h> /* PATH_MAX */
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <vector>
#include "struct_webserv.hpp"
#include "color.hpp"
#include <sys/stat.h>	// pour lstat
#include <dirent.h>		// readdir et struct DIR etc.. (plus besoin si on utilise perl pour générer le html)
#include <unistd.h>		// strlen
#include <cstdlib>
#include <sys/wait.h>
#include <cstring>
#include <fcntl.h>

using namespace std;

class request_handler
{
	// friend cgi_handler;
	typedef int	(request_handler::*t_func)(void);
	typedef std::map<string, string> error_it;

private:
    std::vector<server_info> &			_si; // server_info
	int									_s_id; // server id <- quel server doit répondre à la requete actuelle : '_s_id' est l'index de '_si'
	int									_l_id; // location id <- quel location doit répondre à la requete actuelle : '_l_id' est l'index de 'location' ds '_si'
	std::map<string, vector<string> >	_hrx; // received header_info (requets header info)
	std::map<string, vector<string> >	_htx; // response_Header
	std::map<string, string>			_status; // Response_Status_Msg -> "404" "Not found" etc.
	t_func								_tab[5]; // LookUP table av les fonctions relatives aux types de requete
	std::string							_path; // response_path file
	std::string							_URLquery; // query string contenu ds l'url
	std::string							_body; // response_content (body)
	client_info*						_c;
	// cgi_handler							_cgi;

public:
	request_handler(std::vector<server_info>&);
	~request_handler();
	void reader(client_info& cl_info);
	void writer(void);

	/* FONCTION ACCESSEUR */

	string &get_response(void);

	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

private:
	void gen_date(void);
	void gen_startLine(std::map<string, string>::iterator);
	void gen_serv(void);
	void gen_CType(void);
	void gen_CLength(int);
	// void gen_response(void);

	/* FUNCTION SECONDAIRE : UTILITAIRES */

	void set_server_id(void);
	// void	verify_file_openess(); // Pas utilisé pour le moment
	int	resolve_path();
	void return_directive(vector<locati_info>::reverse_iterator& it);
	bool is_method_allowed(void);
	int	file_type();
	void generate_folder_list();
	size_t check_file_size(void);
	string get_file_size(string& path); // Pas utilisé pour le moment
	void add_all_field();
	void add_body(int i = 0);
	int handle_get_rqst(void);
	int handle_put_rqst(void);	void clean_url(string& str);
	int handle_post_rqst(void);
	int handle_head_rqst(void);
	int handle_delete_rqst(void);
	int extract_multi_rqst_body(void);
	int extract_xform_rqst_body(void);
	int extract_chunk_rqst_body(void);
	void write_file(void);
	void does_file_exist(e_rqst_type);
	/* FUNCTION DE DEBUG */
	
	void display(void);

	/* FUNCTION DE CGI */

	bool	is_cgi();
	int handle_cgi(void);
	int	launch_cgi();
	char**setCGIEnv();
	int cgi_input();
	int cgi_output();
	void clean_body();
};



#endif