/* 
*      HTTP-message   = start-line
*                      *( ./c	er-field CRLF )
*                      CRLF
*                      [ message-body ]
*
*      start-line     = request-line (client)/ status-line (server)

		http://127.0.0.1:8080/ <- c'est ca qui fait bugger
		curl --resolve test_server_block.com:9090:127.0.0.1 http://test_server_block.com:9090/ -X GET
*/

#include "cgi_handler.hpp"
#include "request_handler.hpp"

request_handler::request_handler(std::vector<server_info>& server_info) : _si(server_info)
{// BON EN FAIT IL SEMBLE QUE LA SECU NE SOIT PAS LE POINT IMPORTANT DE WEBSERV...
/* const char *array[] = {	"GET", "Host:", "User-Agent:", "Accept:", "Accept-Language:",
												"Accept-Encoding:", "Connection:", "Upgrade-Insecure-Requests:",
												"Cache-Control:", "DNT:", "rol:"};

	for (size_t i = 0; i < sizeof(array) / sizeof(const char*); ++i)
		_hrx[array[i]];
		// _hrx.insert(std::make_pair(array[i], vector<string>()));
#ifdef _debug_
	for (map< string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it)
		cout <<  YELLOW "map it.first : " RESET << it->first << " size() : " << it->second.size() << endl; 
#endif
*/

// REMPLI LA MAP _STATUS POUR LA STATUS LIGNE (TX) HTML
	ifstream fs("configuration_files/HTML_status_msg.txt");
	if (!fs.is_open()) 
		throw (std::runtime_error("Unkown file : configuration_files/HTML_error_msg.txt"));

	string buf_1, buf_2;
	while (std::getline(fs, buf_1)) {
		if (buf_1[0] == '#' || buf_1[0] == '\0')
			continue ;
		std::stringstream ss_2(buf_1);
		ss_2 >> buf_1; 
		std::getline(ss_2, buf_2);
		_status[buf_1].append(buf_2.substr(0, buf_2.find_first_of('\t')));
	}
#ifdef _debug_
	// for (map< string, string>::iterator it = _status.begin(), end = _status.end(); it != end; ++it)
	// cout <<  YELLOW "map it.first : [" RESET << it->first << "] second : [" << it->second << "]" << endl;
#endif
}

request_handler::~request_handler()
{
}

/* cout << distance(mymap.begin(),mymap.find("198765432")); */ // <- Get index of the pair(key_type, mapped_type) TIPS&TRICKS

void request_handler::reader(client_info& client)
{
	const char	*str = client.rqst.c_str();

	string buf_1, buf_2;
	std::stringstream ss_1(str);
	cout << RED "DANS HEADER READER" RESET "\n" << str << endl;
	// LECTURE DE LA START_LINE
	if (ss_1 && std::getline(ss_1, buf_1)) {
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_1)
			_hrx["A"].push_back(buf_1);
	}
//	for (auto it = _hrx["A"].begin(); it != _hrx["A"].end(); it++)
//		cout << "*it : " << *it << endl;

	// LECTURE DU RESTE DE LA REQUETE
	while (std::getline(ss_1, buf_1)) {
		if (buf_1[0] == '\r') { // SI C'EST UNE REQUESTE POST ON STOCK LE BODY POUR USAGE ULTÉRIEUR
			_hrx["BODY"].resize(1, string());
//			while (std::getline(ss_1, buf_1))
			_hrx["BODY"][0].append(ss_1.str());
			break ;
		}
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_2) {
			// if (_hrx.find(buf_2) != _hrx.end()) {
				string index = buf_2;
				while (ss_2 >> buf_2)
					_hrx[index].push_back(buf_2);
		}
			// }
			// else
			// 	throw (std::runtime_error( "Unkown header field (header_reader) : " + buf_2));
#ifdef _debug_
	// cout << RED << "buf_1.size : "<< buf_1.size() << RESET "[" << buf_1 << "]" << endl;
	// cout << GREEN "ss_2 >> buf_2 : " RESET << "[" << buf_2 << "]" << endl;
	// cout << MAGENTA << "tour" << RESET << endl;
#endif
	}
	set_server_id();
//	client.rqst.clear();
	// cout << RED "APRES GETLINE : " << buf_1 << RESET << endl;
	// this->display();

	std::cout << "=========================================================" << std::endl;
	for(std::map<std::string, std::vector<std::string> >::iterator it = _hrx.begin(); it != _hrx.end(); it++)
	{
		std::cout << it->first << "=";
		for (std::vector<std::string>::iterator i = it->second.begin(); i != it->second.end(); i++)
			std::cout << *i << " ";
		std::cout << std::endl;
	}
	std::cout << "=========================================================" << std::endl;
	
}

void request_handler::writer(void) {
// PAR DEFAULT ON CONSIDÈRE QUE TOUT SE PASSE BIEN ON CHANGE PAR LA SUITE LE STATUS SI UNE EXCEPTION ARRIVE
	gen_startLine( _status.find("200") );
	gen_date();
	gen_serv();

// DÉFINI L'INDEX DE LA LOCATION CONCERNÉE (_l_id) & VÉRIFIE QUE LA MÉTHODE INVOQUÉE Y EST PERMISE
	if (is_cgi(_hrx["A"]))
		handle_cgi();
	else if (resolve_path())
		;
	else if (_hrx["A"][0] == "GET")
		handle_get_rqst();
	else if (_hrx["A"][0] == "POST")
		handle_post_rqst();
	else if (_hrx["A"][0] == "PUT") {
		cout << "Facultatif PUT method not implemented yet\n";
	}
	else if (_hrx["A"][0] == "HEAD") {
		cout << "Facultatif HEAD method not implemented yet\n";
	}
	else if (_hrx["A"][0] == "DELETE") {
		// IF NOT ALLOWED -> 405

		if (_hrx["A"][1][0] == '/')
			_hrx["A"][1].insert(0, ".");
		if( remove( _hrx["A"][1].c_str() ) != 0 ) {
			perror( _hrx["A"][1].c_str() );
			gen_startLine( _status.find("404") ); //  IF NOT FOUND -> 404
		}
		else
			puts( "File successfully deleted" );
	}
/* PROBLEM REDONDANT AVEC LA METHODE GET -> VA FALLOIR CHOISIR*/
	file_type();
	gen_CType(string());
	gen_CLength();
	add_all_field(); 
	add_body();
	_hrx.clear();
	_htx.clear();
}

	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

void	request_handler::gen_startLine(std::map<string, string>::iterator status)
{
	if (_htx["A"].empty())
		_htx["A"] = std::vector<std::string>();
	if (_htx["A"].size() != 3)
		_htx["A"].resize(3, string());
	_htx["A"][0] = "HTTP/1.1 "; // version (static)
	_htx["A"][1] = status->first; // status code (dynamic) -> 200
	_htx["A"][2] = status->second; // status msg (dynamic) -> OK
	_htx["A"][2] += "\r\n";
// MODIFIE LE PNG DS ERROR_PAGE.HTML SI NECESSAIRE
	if (atoi(status->first.c_str()) >= 300) {
		fstream error_file("files/error_pages/error_4xx.html");
		error_file.seekg(1252);
		error_file.write(status->first.c_str(), 3);
		error_file.close();
	}
}

void	request_handler::gen_date()
{
	std::string	date = "Date: ";
	time_t		timer;
	struct tm	*info;
	char		timestamp[36];

	timer = time(NULL);
	info = localtime(&timer);
	strftime(timestamp, 36, "%a, %d %h %Y %H:%M:%S GMT", info);
	date.append(timestamp);
	date.append("\r\n");

	// _response.append(date);
	if (_htx["Date"].empty())
		_htx["Date"] = std::vector<std::string>();
	_htx["Date"].push_back(date);
}

void	request_handler::gen_serv() /* PROBLEM : S'IL N'Y A PAS DE CHAMP Server DS LA REQUETE TU SEGV PAR EX POUR UN POST DS GD TAILLE*/
{
	if (_htx["Server"].size() != 3)
		_htx["Server"].resize(3, string());
	_htx["Server"][0] = "Server: ";// HEADER_LABEL
	if (!_hrx["Host:"].empty())
		_htx["Server"][1] = _hrx["Host:"][0];// IP & HOST
	else
		_htx["Server"][1] = "127.0.0.1";// IP & HOST
	_htx["Server"][2] = "\r\n";
}

// génération du field Content-Type
void	request_handler::gen_CType(string ext) /* PROBLEM : mieux vaudrait extraire ça d'un fichier et le récup ici (serait plus élégant)*/
{
// Capture file.ext(ension)
	if (ext.empty())
		ext = _hrx["A"][1].substr(_hrx["A"][1].find_last_of(".") + 1);
#ifdef _debug_
	cout << "file ext asked : " << ext << endl;
#endif
// Content type : https://developer.mozilla.org/fr/docs/Web/HTTP/Basics_of_HTTP/MIME_types
	_htx["Content-Type"].clear();
	if ( ext == "/" || ext == "html" ) // Default file PROBLEM ?
		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
	else if ( ext == "css" ) // Default file PROBLEM ?
	{
		_htx["Content-Type"].push_back("Content-Type: text/css\r\n");
//		_htx["Connection"].push_back("Connection: close\r\n");
	}
	else if( ext == "ico" || ext == "png" || ext == "jpeg" || ext == "webp" || ext == "gif" || ext == "bmp" )
		_htx["Content-Type"].push_back("Content-Type: image\r\n");
	else if( ext == "ogg" || ext == "wav" || ext == "midi" || ext == "mpeg" || ext == "webm" )
		_htx["Content-Type"].push_back("Content-Type: audio\r\n");
	else if( ext == "ogg" || ext == "mp4" || ext == "webm" )
		_htx["Content-Type"].push_back("Content-Type: video\r\n");
	else // DEFAULT PAREIL QUE L EPREMIER À ARRANGER :) POUR LE TOI DU FUTUR BISOU
		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
		// _htx["Content-Type"].push_back("Content-Type: application/octet-stream\r\n");
}

// génération du field Content-Length et d'un status d'erreur (413) si length > max_file_size
void	request_handler::gen_CLength()
{/* PROBLEME */ // http://127.0.0.1:8080/test les gif 404 s'affiche http://127.0.0.1:8080/test/ <- '/' non
/* PROBLEM */ // http://127.0.0.1:8080/downloads/ n'affiche pas le poulpe -> les autre requetes sont faites sur le folder alors que le path du html est neutre, mais ça ça marche : http://127.0.0.1:8080/downloads 
	_htx["Content-Length"].clear();
	_htx["Content-Length"].push_back("Content-Length: "); // HEADER_LABEL

	stringstream ss;
	if (_body.empty()) {
		ifstream fs(_path.c_str(), std::ifstream::binary | std::ifstream::ate);
		ss << fs.tellg();
		fs.close();
	}
	else
		ss << _body.size();
	_htx["Content-Length"].push_back( ss.str());
	_htx["Content-Length"].push_back( "\r\n"  );
	if (!_si[_s_id].location[_l_id].max_file_size.empty() // POUR LES REQUEST ENTITY TOO LARGE SUREMENT A VIRER D'ICI
		&& atoi(ss.str().c_str()) > atoi(_si[_s_id].location[_l_id].max_file_size.c_str()))
		gen_startLine( _status.find("413") );
}

	/* FUNCTION SECONDAIRE : UTILITAIRES */

// reconnait quel server_virtuel va traiter la requete et initialise _s_id
void request_handler::set_server_id(void)
{
	size_t colon_pos = _hrx["Host:"][0].find_first_of(":");
	string host(_hrx["Host:"][0].substr(0,colon_pos));
	// PROBLEM : SPARADRAP
//	if (host == "127.0.0.1") host = "localhost";
	string port(_hrx["Host:"][0].substr(colon_pos +1));
	for (int i = 0; i < _si.size(); ++i)
		if ( (_si[i].host == host || _si[i].server_name == host) && _si[i].port == port ) {
			_s_id = i;
			cout << RED "_s_id : " RESET << _s_id << endl;
			return ;
		}
// SI LE HOST:PORT N'A PAS ETE TROUVE ON SELECT LE PREMIER SERVER CORRESPONDANT
	for (int i = 0; i < _si.size(); ++i)
		if (_si[i].port == port)
			_s_id = i; // PROBLEM ?
#ifdef _debug_
			cout << "host : " << host << ", port : " << port << ", id : " << _s_id << endl;
#endif
}

/* ◦ Le premier serveur pour un host :port sera le serveur par défaut pour cet
host :port (ce qui signifie qu’il répondra à toutes les requêtes qui n’appar-
tiennent pas à un autre serveur) */



void request_handler::handle_get_rqst(void)
{
	// gen_CType(string());
	// gen_CLength();
	// add_all_field();
	// add_body();
}

void request_handler::handle_post_rqst(void) 
{
	size_t	pos;
	std::string	tmp;

	if (_hrx.find("Content-Type:") != _hrx.end())
		cout << "_hrx['Content-Type:'].size() : " << _hrx["Content-Type:"].size() << " :" << endl;
	for (auto i = _hrx["Content-Type:"].begin(); i != _hrx["Content-Type:"].end(); i++)
		cout << "[" << *i << "]" << endl;

// POUR LIMITER LA TAILLE DU BODY DU CLIENT => JE NE SAIT PAS ENCORE COMMENT GET LA LOCATION CONCERNÉE
	// if (_hrx.find("Content-Length:") != _hrx.end() && atoi(_hrx.find("Content-Length:")->second) > _si[_s_id]. )
	string boundary = _hrx["Content-Type:"][1].substr( _hrx["Content-Type:"][1].find_last_of('-') + 1, string::npos);
	cout << "boundary : " << boundary << endl;
	cout << endl << "BODY : " << endl;
	// for (auto i = _hrx["BODY"].begin(); i != _hrx["BODY"].end(); i++)
	// 	cout << "[" << *i << "]" << endl;
//	cout << _hrx["BODY"][0] << endl;

	// En cas de body plus long qu'autorisé -> 413 (Request Entity Too Large) 
	if (!_si[_s_id].max_file_size.empty() && _hrx["BODY"][0].size() > atoi(_si[_s_id].max_file_size.c_str()) ) {
	std::cout << "YOLO" << std::endl;
		gen_startLine( _status.find("413") ); 
		return ;
	}
	else
	{
		if (!_hrx["BODY"].empty() && !boundary.empty())
		{
			if ((pos = _hrx["BODY"][0].find("\r\n\r\n")) != std::string::npos)
			{
				tmp.append(_hrx["BODY"][0].substr(0, pos), _hrx["BODY"][0].substr(0,pos).length());
				_hrx["BODY"][0] = _hrx["BODY"][0].substr(pos + 4);
			}
			else
				tmp.append(_hrx["BODY"][0]);
			
		}
		if (!tmp.empty() && (pos = tmp.find("filename=\"")) != std::string::npos)
		{
			_hrx["A"][1] = "./files/";
			_hrx["A"][1].append(tmp.substr(pos + strlen("filename=\""), tmp.substr(pos + strlen("filename=\"")).find("\"")));
		}
		std::ofstream	output(_hrx["A"][1]);
		if (!boundary.empty())
		{
				pos = _hrx["BODY"][0].find(boundary);
				if (pos != std::string::npos)
					output << _hrx["BODY"][0].substr(pos);
				else
					output << _hrx["BODY"][0];
		}
		else
		{
			output << _hrx["BODY"][0];
		}
		gen_startLine( _status.find("200") ); 
	}
	// sinon on execute les cgi ?
}

// Permet de séléctionner la location qui partage le plus avec l'url comme le fait nginx,
// si l'url a plusieurs niveau de dossiers, choisi la loc la plus adaptée
// identifie l'index de la location correspondante à l'url spcécifiée (_l_id)
int	request_handler::resolve_path()
{
	size_t len;
#ifdef _debug_
	cout << GREEN "DS RESOLVE_PATH [" + _hrx["A"][1] + "], _s_id : " << _s_id <<  " _path (cleared afterward): " << _path << endl;
#endif
	_path.clear();
// REMOVE MULTIPLE '/' AND THE '/' AT URL'S END
	clean_url(_hrx["A"][1]);
// MANOUCHERIE A VIRER POUR RECUP LES PAGES D'ERREUR SANS BOUCLE SUR 4XX.HTML SI 405
	if (_hrx["A"][1].find("/error_pages/", 0,13) == 0) {
		_path = "files" + _hrx["A"][1];
		return 1;
	}
	if ((len = _path.find("?")) != std::string::npos)
		_path = _path.substr(0, len);
	_l_id = 0;
	int index = _si[_s_id].location.size() - 1;
	len = 0;
	for (vector<locati_info>::reverse_iterator it = _si[_s_id].location.rbegin(); it != _si[_s_id].location.rend(); ++it, --index)
		if ((_hrx["A"][1].find( it->location.c_str(), 0 , it->location.size()) == 0 && it->location.size() > len) || (!len && !index)) // || si on a rien trouvé la location[0] est le default
		{
			// S'IL Y A UNE DIRECTIVE_RETURN À L'INTERIEURE DE LA LOCATION
			if (!it->retour.empty()) {
				for (vector<locati_info>::iterator it2 = _si[_s_id].location.begin(); it2 != _si[_s_id].location.end(); ++it2)
					if (it2->location == it->retour.back()) {
						cout << RED " it->retour[1] :" RESET +  it->retour[1] << endl;
						string::const_iterator c_it = it->retour[0].begin();
						while (c_it != it->retour[0].end() && std::isdigit(*c_it)) ++c_it;
						if ( !it->retour[0].empty() && c_it == it->retour[0].end())
							gen_startLine( _status.find(it->retour[0]) );

						_path = it2->root.back() == '/' ? it2->root : it2->root + "/";
						_l_id = it2 - _si[_s_id].location.begin();
						break ;
					}
			} // SINON
			else {
				_path = it->root.back() == '/' ? it->root : it->root + "/";
				_l_id = index;
			}
			// _path += it->location.back() == '/' ? it->location : it->location + "/";
			_path += _hrx["A"][1].substr(it->location.size());
			len = it->location.length();
			if (it->location.size() == _hrx["A"][1].size())
				_path += _si[_s_id].location[_l_id].index;
		}
#ifdef _debug_
	cout << BLUE "path : " RESET << _path << endl;
#endif
// AFFACER LES '/' EN PRÉFIXE (POUR OUVRIR DEPUIS LA RACINE DE NOTRE DOSSIER ET PAS DEPUIS LA RACINE MERE)
	while (_path[0] == '/')
		_path.erase(_path.begin());
	clean_url(_path);
#ifdef _debug_
	cout << BLUE "resolved_path : " RESET << _path << endl;
	cout << "location [" << _l_id << "] : " + _si[_s_id].location[_l_id].location << endl;
#endif

// VERIFIE SI LA MÉTHODE DS LA LOCATION CONCERNÉE EST AUTORISÉE
	bool allowed = false;
	for (int i = 0; i < _si[_s_id].location[_l_id].allowed_method.size(); ++i)
		if (_si[_s_id].location[_l_id].allowed_method[i] == _hrx["A"][0])
			allowed = true;
	if (!allowed)
		gen_startLine( _status.find("405") );
	return allowed ? 0 : 1; /* PROBLEM  oN SAIT PAS TROP CE QU'ON FAIT LÀ... (double return) */

}

// Détecte si c'est un dossier ou un fichier normal ou s'il n'existe pas (maj de la statut-line si besoin)
// Si erreur lors de l'ouverture du fichier renvoie le _path sur les pages d'erreurs
int request_handler::file_type()
{
	cout << GREEN "DS FILE_TYPE()" RESET <<  " _path : " << _path << endl;
	struct stat sb = {0}; // à la place de : bzero(&sb, sizeof(sb));
	if (lstat(_path.c_str(), &sb) == -1)
		perror("lstat");

	switch (sb.st_mode & S_IFMT) {
		// case S_IFBLK:  printf("block device\n");            break;
		// case S_IFCHR:  printf("character device\n");        break;
		case S_IFDIR:  printf("directory\n");
			if (_si[_s_id].location[_l_id].autoindex == "on") {
// PROBLEM
				generate_folder_list();
				return 1;
			}
			_path = "./files/if_folder.html";                  break;
		// case S_IFIFO:  printf("FIFO/pipe\n");               break;
		// case S_IFLNK:  printf("symlink\n");                 break;
		case S_IFREG:  printf("regular file\n");               break;
		// case S_IFSOCK: printf("socket\n");                  break;
		default:
			if (atoi(_htx["A"][1].c_str()) < 400)
				gen_startLine( _status.find("404") );
			printf(RED "unknown path...\n" RESET);
			break;
	}
	if (atoi(_htx["A"][1].c_str()) >= 400) /* Faire en sorte de changer le png */
		_path = (_si[_s_id].error_page.empty() ? "./files/error_pages/" : _si[_s_id].error_page) + "error_4xx.html";
	printf("_path : %s\n", _path.c_str());
	return 0;
}

// execute le script perl er pipe sont résultat ds _body
void request_handler::generate_folder_list()
{
	int fd[2], pid;
	if (pipe(fd) == -1 || (pid = fork()) == -1)
		throw runtime_error("pipe || fork failed");
	if (pid == 0) {// Child
		char const *argv[] = {"files/cgi/perlFolderLister.pl", _path.c_str(), NULL};
		close(fd[0]);	/* Close unused read end */
		dup2(fd[1], STDOUT_FILENO);
		if (execv(*argv, (char *const *)argv) == -1)
			_exit(EXIT_FAILURE);
		close(fd[1]);
	}
	else {
		close(fd[1]);          /* Close unused write end */
		char buf[1000];
		int n;
		_body.clear();
		while ((n = read(fd[0], buf, 999))) {
			buf[n] = '\0';
			_body.append(buf);
		}
		gen_CType("html");
		// gen_CType("html");
		cout <<  CYAN "folder_ response" RESET << _body  <<  CYAN "path_ response" RESET << _path << endl;
		close(fd[0]);          /* Reader will see EOF */
		wait(NULL);            /* Wait for child */
		return ;
	}
}

// Clear la string (response) et y ajoute tous les field, puis clear _hrx
void request_handler::add_all_field()
{
	_response.clear();
	for (std::map<string, vector<string> >::iterator it = _htx.begin(); it != _htx.end(); it++)
		for (size_t i = 0, j = it->second.size(); i < j; ++i)
			_response += it->second[i];
	_response += "\r\n";
	cout << BLUE "Response Headers (add_all_field()) :\n" RESET << _response << endl;
}

// Ajout du fichier ou du body À LA SUITE des header dans response
void request_handler::add_body()
{
	if (_htx["A"][1] == "413")
		return ;
	if (!_body.empty()) {
		_response += _body;
		_body.clear();
		return ;
	}
// S'IL S'AGIT D'UN GET OU D'UN POST ON JOINS LE FICHIER
	if (_hrx["A"][0] == "GET" || _hrx["A"][0] == "POST") {
		cout << RED "File written !" RESET  << endl;
		ifstream fs(_path);
		_response.append((istreambuf_iterator<char>(fs)),
						 (istreambuf_iterator<char>() ));
	}
}

// Remove multiple '/' and the '/' at url's end
void request_handler::clean_url(string& str)
{
	for (size_t i = 0; i < str.size(); ++i)
		while (str[i] == '/' && (str[i + 1] == '/' || str[i + 1] == '\0') && str.size() > 1)
			str.erase(i, 1);
}


	/* FUNCTION GETTER / SETTER */

std::string &request_handler::get_response(void) {return _response;}

	/* FUNCTION DE DEBUG */

void request_handler::display(void) {
	cout << GREEN ITALIC UNDERLINE "DISPLAY HEADER INFORMATION" RESET GREEN " :" RESET << endl;
	for (map<string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it) {
		cout << it->first << " ";
		for (size_t i = 0; i < it->second.size(); ++i)
			cout << it->second[i] << ", ";
		cout << endl;
	}
}

void	request_handler::handle_cgi(void)
{
		size_t	pos;
		int		bfd[2];

		//HERE!
		resolve_path();
		_hrx.insert(std::make_pair("query", std::vector<std::string>()));
		_hrx["query"].push_back(_path);
		if (_s_id == -1)
		{
			std::cout << "Invalid server id: " << _s_id << std::endl;
			return ;
		}
		else
		{
			bfd[0] = dup(STDIN_FILENO);
			bfd[1] = dup(STDOUT_FILENO);
			go_cgi(_hrx, _si[_s_id], STDIN_FILENO);
			dup2(bfd[0], STDIN_FILENO);
			dup2(bfd[1], STDOUT_FILENO);
			close(bfd[0]);
			close(bfd[1]);
	//		dup2(STDOUT_FILENO, bfd[1]);
	//		dup2(STDIN_FILENO, bfd[0]);
		}
		_response.clear();
		_response += (char*)"HTTP/1.1 200 OK\r\n";
		for (size_t i = 0, j = _hrx["A"].size(); i < j; i++)
		{
			_response += _hrx["A"][i];
		}
		_response += "Status: 200 Success\r\n";
		_response += "Pragma: no-cache\r\n";
//		_response += "Location:\r\n";
		size_t k = 0;
		if (!_hrx["BODY"].empty())
		{
			for (size_t i = 0, j = _hrx["BODY"].size(); i < j; i++)
			{
//				std::cout << _hrx["BODY"][i] << std::endl;
				if (!_hrx["BODY"][i].empty())
					k += _hrx["BODY"][i].size() + 1;
			}
//			_response += "Connection: close\r\n";
		}
		_response += "Content-Lenght: ";
		_response += std::to_string(k - 1);
		_response += "\r\n";
		if (!_hrx["Content-Type"].empty())
		{
			_response += "Content-Type: ";
			for (size_t i = 0, j = _hrx["Content-Type"].size(); i < j; ++i)
				_response += _hrx["Content-Type"][i];
	//		_response += "\r\n";
		}
		_response += "Content-Language: en\r\n";
		if (!_htx["Date"].empty())
		{
			for (size_t i = 0, j = _htx["Date"].size(); i < j; i++)
			{
				_response += _htx["Date"][i];
			}
			_response += "Last-Modified:";
			for (size_t i = 0, j = _htx["Date"].size(); i < j; i++)
			{
				if ((pos = _htx["Date"][i].find("Date:")) != std::string::npos)
					_response += _htx["Date"][i].substr(pos + 5);
				else
					_response += _htx["Date"][i];
			}
		}
		if (!_htx["Server"].empty())
		{
			for (size_t i = 0, j = _htx["Server"].size(); i < j; i++)
			{
				_response += _htx["Server"][i];
			}
		}
		_response += "\r\n";
		if (!_hrx["BODY"].empty())
		{
			for (size_t i = 0, j = _hrx["BODY"].size(); i < j; ++i)
			{
			//	std::cout << _hrx["BODY"][i];
				if (!_hrx["BODY"][i].empty() && _hrx["BODY"][i][0] != '\r')
				{
					_response += _hrx["BODY"][i];
					_response += "\r\n";
				}
			}
		}
//		_response += "\r\n";
		cout << RED "Response :\n" RESET << _response << endl;
}

/* recursive path solver(42 syntax)

void	request_handler::resolve_path(string& path)
{
// REMOVE TRAILING '/' AT URL'S END
//	while (_hrx["A"][1].back() == '/' && _hrx["A"][1].size() != 1)
//		_hrx["A"][1].pop_back();

	size_t pos;
	string url, uri;

	if (_hrx["A"][1] == "/") {
		cout << "ds if\n";
		path =  "./" + _si[_s_id].location["/"].root + "/" + _si[_s_id].location["/"].index;
	}
	else { // SPLIT URL (path...) AND URI POUR RECHERCHE DS LES "LOCATION" DU SERVER CONCERNÉ
		cout << "ds else\n";
		pos = _hrx["A"][1].find("/");
		if (pos != std::string::npos)
		{
			url = _hrx["A"][1].substr(0, pos + 1);
			uri = _hrx["A"][1].substr(pos + 1);
			location_lookup(path, url, uri, _si[_s_id].location);
		}
	}
	cout << BLUE "url : " RESET << url << BLUE ", uri : " RESET << uri << BLUE ", path : " RESET << path << endl;

	if (path.empty()) {
		path = "./" + (_si[_s_id].location["/"].root.back() == '/' ? _si[_s_id].location["/"].root : _si[_s_id].location["/"].root + "/");
		// path += url; // c'est ici que "/test" se mets ds le path
		path += uri;
	}

	cout << BLUE "url : " RESET << url << BLUE ", uri : " RESET << uri << BLUE ", path : " RESET << path << endl;

	pos = path.find(".php");
	if (pos != std::string::npos)
		path = path.substr(0, pos + 4);
	int file_tp = file_type(path, uri);

#ifdef _debug_
	cout << BLUE "resolved_path : " RESET << path << " uri(" + uri + ")" << endl;
#endif
}

// si l'url a plusieurs niveau de dossiers : choisir le plus adapté
int request_handler::location_lookup(string& path, string url, string uri, std::map<std::string, locati_info>& loc)
{
	size_t	pos;

	cout << BLUE "url : " RESET << url << BLUE " uri : " RESET << uri << endl;
	if (loc.find(url) != loc.end())
	{
		location_lookup(path, url, uri, loc[url].location);
	}
	else if ((pos = uri.find("/")) != std::string::npos)
	{
		url = uri.substr(0, pos + 1);
		uri = uri.substr(pos + 1);
		location_lookup(path, url, uri, loc);
	}
	if (path.empty())
	{
		if (url != "/" && loc.find(url) != loc.end())
		{
			path = loc[url].root;
			if (path.back() != '/')
				path += "/";
			path += uri;
		}
	}
	return (0);
}
*/
