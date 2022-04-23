/* 
*      HTTP-message   = start-line
*                      *( header-field CRLF )
*                      CRLF
*                      [ message-body ]
*
*      start-line     = request-line (client)/ status-line (server)

		http://127.0.0.1:8080/ <- c'est ca qui fait bugger

		// https://everything.curl.dev/http/post/chunked  // Site web pour curl
		curl --resolve test_server_block.com:9090:127.0.0.1 http://test_server_block.com:9090/ -X GET // Change le server_name
		curl -X POST -F 'create_to_erase=@Makefile' 127.0.0.1:8081/(create_to_erase) -v // Fait une post request
		curl -T Makefile 127.0.0.1:8080/new/resource/file // Veritable put selon Site web pour curl
		curl -X PUT -F 'file=@compil.sh' 127.0.0.1:8080/create_to_erase -v
		curl -H "Transfer-Encoding: chunked" -d @file http://example.com

*/


#include "request_handler.hpp"

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

request_handler::request_handler(std::vector<server_info>& server_info) : _si(server_info)
{
// CREER LA LOOKUP TABLE
	_tab[0] = &request_handler::handle_get_rqst;
	_tab[1]	= &request_handler::handle_put_rqst;
	_tab[2]	= &request_handler::handle_post_rqst;
	_tab[3]	= &request_handler::handle_head_rqst;
	_tab[4]	= &request_handler::handle_delete_rqst;

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
	// cgi_handler							_cgi(*this);

#ifdef _debug_
	// for (map< string, string>::iterator it = _status.begin(), end = _status.end(); it != end; ++it)
	// cout <<  YELLOW "map it.first : [" RESET << it->first << "] second : [" << it->second << "]" << endl;
#endif
}

request_handler::~request_handler()
{
}

/* cout << distance(mymap.begin(),mymap.find("198765432")); */ // <- Get index of the pair(key_type, mapped_type) TIPS&TRICKS

void request_handler::reader(client_info& cl_info)
{
	string buf_1, buf_2;
	_c = &cl_info;
	std::stringstream ss_1(cl_info.rqst);
#ifdef _debug_
	cout << RED "DANS HEADER READER" RESET "\n" << endl; //cl_info.rqst << endl;
	cout << "[" + _c->rqst.substr(0, _c->header_end) << "]\n";
#endif
	// LECTURE DE LA START_LINE
	if (std::getline(ss_1, buf_1)) {
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_1)
			_hrx["A"].push_back(buf_1);
	}
	// LECTURE DU RESTE DE LA REQUETE
	while (std::getline(ss_1, buf_1)) {
		if (buf_1[0] == '\r') { 
			// 3eme SOLUTION : SI C'EST UNE REQUESTE POST OU PUT ON IGNORE LE BODY POUR USAGE ULTÉRIEUR AV _c
			break ;
		}
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_2) {
			// if (_hrx.find(buf_2) != _hrx.end()) { // VERIFICATION DE LA SYNTAXE DES HEADERS AU CAS OU L'ON VEUILLE FAIRE DE LA SÉCURITÉ...
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
	
#ifdef _log_
	ofstream _LOGfile("log.txt", std::ofstream::app);
	if (_LOGfile.is_open()) {
		_LOGfile << _c->rqst.substr(0, _c->header_end) << "\n";
		_LOGfile.close();
	}
#endif

		// this->display();
}

void request_handler::writer(void) {

#ifdef _debug_
	cout << RED "DANS HEADER WRITER" RESET << endl;
#endif

// PAR DEFAULT ON CONSIDÈRE QUE TOUT SE PASSE BIEN ON CHANGE PAR LA SUITE LE STATUS SI UNE EXCEPTION ARRIVE
	gen_startLine( _status.find("200") );
	gen_date();
	gen_serv();

/* 	static int i = 0;
	static int cpt = 0;
	if (i == _c->rqst_t)
		cout << cpt++ << "\r";
	i = _c->rqst_t; */

// EN CAS DE MÉTHODE NON AUTORISÉE DS CETTE LOCATION ON ENVOI LES PQGES D'ERREUR
	if (resolve_path()) {
		file_type();
		gen_CLength(2);
		add_all_field();
		add_body(false);
	}
	else if (is_cgi()) {
		if (handle_cgi()) { // SI erreur alors on passe par file_type() , add_al...etc.
			file_type();
			gen_CLength(2);
			add_all_field();
			add_body(false);
		}
	}
	else (this->*_tab[static_cast<int>(_c->rqst_t)])();
	_body.clear();
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
		fstream error_file("files/error_pages/4xx.html");
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

	if (_htx["Date"].empty())
		_htx["Date"] = std::vector<std::string>();
	_htx["Date"].push_back(date);
}

void	request_handler::gen_serv()
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

void	request_handler::gen_CType(/* string ext */) /* PROBLEM : mieux vaudrait extraire ça d'un fichier et le récup ici (serait plus élégant)*/
{

	string ext = _path.substr(_path.find_last_of(".") + 1);
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

// génération du header Content-Length (1 = clen, 2 = file_path, 3 = _body.size())
void	request_handler::gen_CLength(int slct_src_len)
{/* PROBLEME */ // http://127.0.0.1:8080/test les gif 404 s'affiche http://127.0.0.1:8080/test/ <- '/' non
/* PROBLEM */ // http://127.0.0.1:8080/downloads/ n'affiche pas le poulpe -> les autre requetes sont faites sur le folder alors que le path du html est neutre, mais ça ça marche : http://127.0.0.1:8080/downloads 

#ifdef _debug_
	cout << BLUE "IN GEN_CLENGTH()" RESET << endl;
#endif
	_htx["Content-Length"].clear();

	stringstream ss;
	if (slct_src_len == 1) {
		ss << _c->clen;
#ifdef _debug_
		cout << "add c->clen(" << _c->clen << ") (pour les chunck) vs _body.size() = " <<_body.size() << "\n";
#endif
	}
	else if (slct_src_len == 2) {
		ifstream fs(_path.c_str(), std::ifstream::binary | std::ifstream::ate);
		ss << fs.tellg();
#ifdef _debug_
		cout << "add file.size(" << fs.tellg() << ")\n";
#endif
		fs.close();
	}
	else {
		ss << _body.size();
#ifdef _debug_
		cout << "add _body.size(" << _body.size() << ")\n";
#endif
	}
	_htx["Content-Length"].push_back("Content-Length: "); // HEADER_LABEL -> IL APPARAITTRA ALORS DS LES HEADER _HTX
	_htx["Content-Length"].push_back( ss.str());
	_htx["Content-Length"].push_back( "\r\n"  );
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
	for (size_t i = 0; i < _si.size(); ++i)
		if ( (_si[i].host == host || _si[i].server_name == host) && _si[i].port == port ) {
			_s_id = i;
			_c->serv = &_si[i]; // nvx ds la struct client_info
#ifdef _debug_
			cout << RED "_s_id : " RESET << _s_id << endl;
#endif
			return ;
		}
// SI LE HOST:PORT N'A PAS ETE TROUVE ON SELECT LE PREMIER SERVER CORRESPONDANT
	for (size_t i = 0; i < _si.size(); ++i)
		if (_si[i].port == port) {
			_s_id = i;
			_c->serv = &_si[i]; // nvx ds la struct client_info
			break;
		}
#ifdef _debug_
			cout << "host : " << host << ", port : " << port << ", id : " << _s_id << endl;
#endif
}

/* ◦ Le premier serveur pour un host :port sera le serveur par défaut pour cet
host :port (ce qui signifie qu’il répondra à toutes les requêtes qui n’appar-
tiennent pas à un autre serveur) */



int request_handler::handle_get_rqst(void)
{
#ifdef _debug_
	cout << BLUE "IN HANDLE_GET_RQST" RESET << endl;
#endif
	int fileType = file_type();
	if (!fileType)
		gen_CType();
	gen_CLength(2);
	add_all_field();
	add_body(fileType); // add file content
	return 0;
}

int request_handler::handle_put_rqst(void)
{
#ifdef _debug_
	// EN CAS DE BODY PLUS LONG QU'AUTORISÉ -> 413 (REQUEST ENTITY TOO LARGE) MAL GERÉ PROBLEM
	cout << BLUE "IN HANDLE_PUT_RQST" RESET << endl;
#endif

// PROBLEM : NO CHECK NO GOOD (POUR LES VALEURS RETOURNÉES PAR LES FIND)
	if (_c->rqst_transfer_t == MULTIPART) {
		// extract_multi_rqst_body();
// NGINX ECRIT UN PUT EN MULTIPART AVEC SES BOUNDARY SANS LE TRAITER DS LE FICHIER
		size_t actual_file_size;
		if ((actual_file_size = check_file_size()) == string::npos) // PROBLEM RETURN 1 COMPLETEMENT INUTILE COMME ÇA !! DOIT ETRE REVU
			return 1;
		size_t pos_boundary;
		if ((pos_boundary = _c->rqst.find("\r\n\r\n")) == string::npos) // Vérifie qu'on a au moins les headers
			return 1;
		pos_boundary += 4;
#ifdef _debug_
		cout << MAGENTA "_c->rqst.size() / pos_boundary : " RESET << _c->rqst.size() << " / " << pos_boundary <<endl;
		cout << MAGENTA "actual_file_size : " RESET << actual_file_size << endl;
		if (_c->rqst.size() < 1000)
			cout << MAGENTA "_c->rqst : " RESET << _c->rqst << endl;
#endif
		if ( (_c->rqst.size() - pos_boundary) < actual_file_size )
			return 1;
#ifdef _debug_
		cout << MAGENTA "LA\n" RESET;
#endif

	_body = _c->rqst.substr(pos_boundary);
	}
	else if (_c->rqst_transfer_t == URL_ENCODED)
		extract_xform_rqst_body();
	else if (_c->rqst_transfer_t == CHUNCK) {
		cout << BLUE "CHUNCK" RESET << endl;
		if (extract_chunk_rqst_body())
			cout << RED << "ERROR PUT CHUNK\n" RESET;
	}

	if (atoi(_htx["A"][1].c_str()) < 300) {
		does_file_exist(PUT);
		write_file();
	}
	// gen_CType();
	// gen_CLength(3);
	add_all_field();
	if (atoi(_htx["A"][1].c_str()) >= 300){ // S'il y a un erreur mais pas bien fait PROBLEM
		file_type();
		add_body();
	}
	return 0;
}

int request_handler::handle_post_rqst(void) 
{
#ifdef _debug_
	cout << BLUE "IN HANDLE_POST_RQST" RESET << endl;
	// cout << "_hrx['Content-Type:''][1] : " << _hrx["Content-Type:"][0] << endl;
	if (_c->rqst.size() < 1000)
		cout << GREEN "RQST :\n" RESET << "[" + _c->rqst + "]" << endl;
#endif
// PROBLEM SI C URL_ENCODE C DS LE FIELD  _hrx["Content-Type:"][0] SINON LA BOUNDARY EST DE LE FIELD _hrx["Content-Type:"][1] D'OU LE SEGV plsu bas si pas de boundary

	// SI TOUT S'EST BIEN PASSÉ ON DÉTACHE LE BODY DE LA REQUETE POST (DS _BODY)
	if (_c->rqst_transfer_t == MULTIPART)
		extract_multi_rqst_body();
	else if (_c->rqst_transfer_t == URL_ENCODED) // Content-Type: application/x-www-form-urlencoded
		extract_xform_rqst_body();
	else if (_c->rqst_transfer_t == CHUNCK)
		extract_chunk_rqst_body();

	if (atoi(_htx["A"][1].c_str()) < 300) {
		does_file_exist(POST);
		write_file();
		if (_c->rqst_transfer_t == CHUNCK)
			gen_CLength(1);
	}

	if (atoi(_htx["A"][1].c_str()) >= 300){ // S'il y a un erreur mais pas bien fait PROBLEM
		file_type();
		gen_CLength(2);
	}
	add_all_field();
	if (atoi(_htx["A"][1].c_str()) >= 300) // S'il y a un erreur mais pas bien fait PROBLEM
		add_body();
	else	
		add_body(true);

	return 0;
}

int request_handler::handle_head_rqst()
{
#ifdef _debug_
	cout << BLUE "IN HANDLE_HEAD_RQST" RESET << endl;
#endif

	file_type();
	gen_CType();
	gen_CLength(2);
	add_all_field();
	// gen_startLine( _status.find("405") ); // 405 Not allowed - 403 Forbidden
	return 0;
}

int request_handler::handle_delete_rqst()
{
		// IF NOT ALLOWED -> 405
#ifdef _debug_
	cout << BLUE "IN HANDLE_DELETE_RQST" RESET << endl;
#endif

	if (_hrx["A"][1][0] == '/')
		_hrx["A"][1].insert(0, ".");
	if( remove( _hrx["A"][1].c_str() ) != 0 ) {
		perror( _hrx["A"][1].c_str() );
		gen_startLine( _status.find("404") ); //  IF NOT FOUND -> 404
	}
	else {
		gen_startLine( _status.find("204") );
		puts( "File successfully deleted" );
	}
	return 0;
}


// Extrait le body du message de la post request et le mets ds le _body pour la reponse
int request_handler::extract_multi_rqst_body(void)
{

// PROBLEM SI C URL_ENCODED C DS LE FIELD  _hrx["Content-Type:"][0] SINON LA BOUNDARY EST DS LE FIELD _hrx["Content-Type:"][1] D'OU LE SEGV plsu bas si pas de boundary
	string boundary = _hrx["Content-Type:"][1].substr( _hrx["Content-Type:"][1].find_last_of('-') + 1, string::npos);
#ifdef _debug_
	cout << "boundary : " << boundary << endl;
	// cout << endl << "_hrx['Content-Length'][0].c_str() : " << _hrx["Content-Length:"][0].c_str() << endl;
#endif

	// EN CAS DE BODY PLUS LONG QU'AUTORISÉ -> 413 (REQUEST ENTITY TOO LARGE)
	if (check_file_size() == string::npos) // PROBLEM NE SERT A RIEN ICI -> RETURN INUTILE
		return 1;

	client_info& cl = *_c;
	size_t pos, pos1;
// PROBLEM : NO CHECK NO GOOD (POUR LES VALEURS RETOURNÉES PAR LES FIND)
	size_t pos_boundary;
	if ((pos_boundary = cl.rqst.find("\r\n\r\n")) == string::npos) // Vérifie qu'on a au moins les headers
		return 1;
	pos_boundary += 4;
	size_t pos_last_boundary = cl.rqst.find_last_of("\r\n", cl.rqst.size() - 4);

// GET NAME AND FILENAME INSIDE BOUNDARY
	string name("name=\"");
	string filename("filename=\"");
	// cout << "name [" << name + "]" << endl;
	if ( (pos = cl.rqst.find(name, pos_boundary)) != string::npos )
		if ( (pos1 = cl.rqst.find_first_of('"', pos + name.size())) != string::npos )
			name = cl.rqst.substr(pos + name.size(), pos1 - (pos + name.size()));
#ifdef _debug_
	cout << "pos " << pos << " pos1 " << pos1 <<  " name [" << name + "]" << endl;
#endif

	// cout << "filename [" << filename + "]" << endl;
	if ( (pos = cl.rqst.find(filename, pos_boundary)) != string::npos )
		if ( (pos1 = cl.rqst.find_first_of('"', pos + filename.size())) != string::npos )
			filename = cl.rqst.substr(pos + filename.size(), pos1 - (pos + filename.size()));
#ifdef _debug_
	cout << "pos " << pos << " pos1 " << pos1 <<  " filename [" << filename + "]" << endl;
#endif
// END GET NAME AND FILENAME INSIDE BOUNDARY

// ECRIT DS _BODY POUR LA REPONSE !! NE CREER PLUS LE FICHIER !!
	cl.post_file_path = _path + '/' + name + "_transfer";
	if ((pos = cl.rqst.find("\r\n\r\n", pos1)) != string::npos && (pos += 4)) //{// +=4 == "\r\n\r\n"
		_body = cl.rqst.substr(pos, pos_last_boundary - pos - 1);
/* 		ofstream my_file(cl.post_file_path, std::ofstream::out | std::ofstream::trunc);
		if (my_file.is_open()) {
			my_file << _body;
			my_file.close();
			cout << "Upload location : " + _path + '/' + name + "_transfer\n\n" + cl.rqst + "\n\n";
		}
		else
			cout << RED "download path invalid : " RESET << cl.post_file_path << "\n";
	} */
	return 0;
}

// Extrait le body des requetes de type : Content-Type: application/x-www-form-urlencoded
// où le query string est représenté par le body et le met ds _URLquery
int request_handler::extract_xform_rqst_body(void)
{// https://developer.mozilla.org/fr/docs/Web/HTTP/Methods/POST
#ifdef _debug_
	cout << BLUE "\nDS extract_xform_rqst_body\n" RESET;
#endif

	client_info& cl = *_c;
	size_t pos_boundary;

	// EN CAS DE BODY PLUS LONG QU'AUTORISÉ -> 413 (REQUEST ENTITY TOO LARGE)
	if (check_file_size() == string::npos) // PROBLEM NE SERT A RIEN ICI -> RETURN INUTILE
		return 1;

	if ((pos_boundary = cl.rqst.find("\r\n\r\n")) == string::npos) // Vérifie qu'on a au moins les headers
		return 1;
	pos_boundary += 4;
	size_t pos_last_boundary = cl.rqst.find_last_of("\r\n", cl.rqst.size() - 4);
	_URLquery = cl.rqst.substr(pos_boundary, pos_last_boundary - pos_boundary - 1);
#ifdef _debug_
	if (_URLquery.size() <= 1000)
		cout << "[" << _URLquery << "]" << endl;
	else 
		cout << RED "Body too long to be displayed\n" RESET << endl;
#endif
	return 0;
}

int request_handler::extract_chunk_rqst_body(void)
{
#ifdef _debug_
	cout << BLUE "DS EXTRACT_CHUNK_RQST_BODY\n" RESET;
#endif
	_body.clear();
	size_t pos, count = 1;
	if ((pos = _c->rqst.find("\r\n\r\n")) == string::npos) // Vérifie qu'on a au moins les headers
		return 1;
	pos += 2;
	while (count) {
		pos += 2;
		_c->clen += count = strtol(&_c->rqst[pos], NULL, 16);
#ifdef _debug_
		cout << "pos : " << pos << ", count : " << count << endl;
#endif
		pos = _c->rqst.find_first_of("\n", pos) + 1;
		_body.append(_c->rqst, pos, count);
		pos += count;
	}
#ifdef _debug_
	cout << "here is the concatenated string size : " << _body.size()  << endl;
#endif

	if (check_file_size() == string::npos) {// PROBLEM NE SERT A RIEN ICI -> RETURN INUTILE
		cout << "rqst file size trop grande\n";
		return 1;
	}

#ifdef _debug_
	if (_body.size() <= 1000)
		cout << "[" << _body << "]" << endl;
	else 
		cout << RED "Body too long to be displayed\n" RESET << endl;
#endif
	return 0;
}

// Permet de séléctionner la location qui partage le plus avec l'url,
// identifie l'index de la location correspondante à l'url spcécifiée (_l_id)
// Verifie si la methode est autorisee (maj gen_stratLine 405 ou 403 si besoin)
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
// ON ENLEVE LES ARGUMENTS S'IL Y EN A DS L'URL
	if ((len = _path.find("?")) != std::string::npos)
		_path = _path.substr(0, len);

	_l_id = 0;
	int index = _si[_s_id].location.size() - 1;
	len = 0;
	for (vector<locati_info>::reverse_iterator it = _si[_s_id].location.rbegin(); it != _si[_s_id].location.rend(); ++it, --index)
		if ((_hrx["A"][1].find( it->location.c_str(), 0 , it->location.size()) == 0 && it->location.size() > len) || (!len && !index)) // || si on a rien trouvé la location[0] est le default
		{
			// S'IL Y A UNE DIRECTIVE_RETURN À L'INTERIEURE DE LA LOCATION
			if (!it->retour.empty())
				return_directive(it);
			// SINON
			else {
				_path = it->root + "/";
			// SI POST || PUT ET PRESENCE DIRECTIVE_DOWNLOAD À L'INTERIEURE DE LA LOCATION
				if ((_c->rqst_t == POST || _c->rqst_t == PUT) && !it->download_path.empty())
					_path = it->download_path + '/'; // on prend le path_ de download_path tel quel (pas de combinaison av root mettre += si on veut le combiner)
				_l_id = index;
				_c->loc = &_si[_s_id].location[_l_id]; // nvx ds la struct client_info
			}
			// _path += it->location.back() == '/' ? it->location : it->location + "/";
			_path += _hrx["A"][1].substr(it->location.size());
			len = it->location.length();
			// if (it->location.size() == _hrx["A"][1].size()) // Mnt ajout de l'index.html mis dorenavant ds file_type si necessaire
			// 	_path += _si[_s_id].location[_l_id].index;
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
// VERIFIE SI LA MÉTHODE DS LA LOCATION CONCERNÉE EST AUTORISÉE (maj gen_stratLine 405 si besoin)
	return !is_method_allowed(); // retourne 1 si allowed d'ou le '!'
}

// Pour Resolve_path() : s'il y a une directive_return à l'interieure de la location
void request_handler::return_directive(vector<locati_info>::reverse_iterator& it)
{
	for (vector<locati_info>::iterator it2 = _si[_s_id].location.begin(); it2 != _si[_s_id].location.end(); ++it2)
		if (it2->location == it->retour.back()) {
#ifdef _debug_
			cout << RED " it->retour[1] :" RESET +  it->retour[1] << endl;
#endif
			string::const_iterator c_it = it->retour[0].begin();
			while (c_it != it->retour[0].end() && std::isdigit(*c_it))  // On verifie qu'il s'agisse slmt de chiffre ...
				++c_it;
			if ( !it->retour[0].empty() && c_it == it->retour[0].end()) // ... d'un status_code ds it->retour[0]
				gen_startLine( _status.find(it->retour[0]) ); // 301 dependament du .conf

			_path = it2->root + "/";
			_l_id = it2 - _si[_s_id].location.begin();
			_c->loc = &_si[_s_id].location[_l_id];  // nvx ds la struct client_info
			break ;
		}
}

// Détecte si c'est un fichier normal ou s'il n'existe pas (maj de la statut-line si besoin)
// Si c'est un dossier check l'autoindex ou la presence d'une directive index et affiche le contenu du dossier ou ajoute le fichier index respectivement
// Si erreur lors de l'ouverture du fichier renvoie le _path sur les pages d'erreurs
int request_handler::file_type()
{
#ifdef _debug_
	cout << BLUE "DS FILE_TYPE()" RESET <<  " _path : " << _path << endl;
#endif
	// struct stat sb = {0}; // à la place de : bzero(&sb, sizeof(sb));
	struct stat sb;
	bzero(&sb, sizeof(sb));
	if (lstat(_path.c_str(), &sb) == -1)
		perror("lstat");

	switch (sb.st_mode & S_IFMT) {
		case S_IFDIR:
#ifdef _debug_
			printf(RED "directory\n" RESET);
#endif
			if (_si[_s_id].location[_l_id].autoindex == "on") {
				generate_folder_list();
				return 1;
			}
// S'IL S'AGIT D'UN DOSSIER DS LEQUEL IL Y A UN INDEX.HTML A RÉCUPÉRER
			if (!_si[_s_id].location[_l_id].index.empty()) {
				_path += /* _path.back() == '/' ? _si[_s_id].location[_l_id].index : */ '/' + _si[_s_id].location[_l_id].index;
				file_type();
			}
			break;

		case S_IFREG:
#ifdef _debug_
			printf(RED "regular file\n" RESET);
#endif
			break;

		default:
			if (atoi(_htx["A"][1].c_str()) < 400)
				gen_startLine( _status.find("404") );
			printf(RED "unknown path : %s\n" RESET, _path.c_str());
			break;
	}
// SI ERROR, AIGUILLE LE PATH SUR LA PAGE D'ERREUR CORRESPONDANTE
	if (atoi(_htx["A"][1].c_str()) >= 400)
		_path = _si[_s_id].error_page.empty() ? "files/error_pages/4xx.html" : _si[_s_id].error_page + "/" + _htx["A"][1] + ".html";
#ifdef _debug_
	printf("_path : %s\n", _path.c_str());
#endif
	return 0;
}

// Créé la liste de fichier(s) à afficher ds _body
void request_handler::generate_folder_list()
{
	DIR *dpdf;
	set<string> st;
	struct dirent *epdf;

	dpdf = opendir(_path.c_str());
	if (dpdf != NULL)
	   	while ((epdf = readdir(dpdf))) {
			st.insert(epdf->d_name);
#ifdef _debug_
			std::cout << epdf->d_name << std::endl;
#endif
		}
	closedir(dpdf);

	fstream autoindex_file("configuration_files/autoindex.html");
	if (!autoindex_file.is_open())
		throw (std::runtime_error("Couldn't open : configuration_files/autoindex.html"));
// BUFFERISE LE TEMPLATE HTML POUR Y AJOUTER LE CONTENU DU DOSSIER
	std::stringstream buffer;
	buffer << autoindex_file.rdbuf();
	_body = buffer.str();
	autoindex_file.close();
	for (set<string>::iterator i = st.begin(); i != st.end(); ++i)
		_body.append("<div style='padding:10px;'><a href=\"" + _hrx["A"][1] + '/' + *i + "\">" + *i + "</a></div>");
	_body.append("</div></body></html>");
}

// Clear la string _c->resp (response) et y ajoute tous les field
void request_handler::add_all_field()
{
	_c->resp.clear();
	for (std::map<string, vector<string> >::iterator it = _htx.begin(); it != _htx.end(); it++)
		for (size_t i = 0, j = it->second.size(); i < j; ++i)
			_c->resp += it->second[i];
	_c->resp += "Connection: close\r\n"; // NE SERT A RIEN MANIFESTEMENT
	_c->resp += "\r\n";
#ifdef _debug_
	if (_c->rqst.size() < 1000)
		cout << BLUE "DS ADD_ALL_FIELD() all_response_header :\n" RESET << _c->resp << endl;
#endif

#ifdef _log_
	ofstream _LOGfile;
	_LOGfile.open("log.txt", std::ofstream::app);
	if (_LOGfile.is_open()) {
		_LOGfile << "Response :\n";
		string str;
		std::stringstream ss(_c->resp);
		while (getline(ss, str))
			_LOGfile << "> " + str + "\n";
		_LOGfile << "\n";
		_LOGfile.close();
	}
#endif
}

// Ajout du fichier ou du body À LA SUITE des header dans response (src == 0 ->use _path, src == value -> use _body)
void request_handler::add_body(int slt_src)
{
#ifdef _debug_
	if (_c->rqst.size() < 1000)
		cout << BLUE "DS ADD_BODY() body :\n" RESET << _body << endl;
	else 
		cout << BLUE "DS ADD_BODY() body(TOO_LONG)size : " RESET << _body.size() << endl;
#endif
	if (slt_src) {
#ifdef _debug_
		cout << RED "Body (folder) written !" RESET  << endl;
#endif
		_c->resp += _body;
		_body.clear();
		return ;
	}
// S'IL S'AGIT D'UN GET OU D'UN POST ON JOINS LE FICHIER
	else {
#ifdef _debug_
		cout << RED "File written !" RESET  << endl;
#endif
		ifstream fs(_path.c_str());
		_c->resp.append((istreambuf_iterator<char>(fs)),
						(istreambuf_iterator<char>() ));
	}
}

// Verifie si la méhtode est autorisee ds le cas d'une methode inconnue faite av curl -X (maj gen_stratLine 405 si besoin)
// Set _c->rqst_t dependanmment du type de requete
// puis si la méthode ds la location concernée est autorisée (maj gen_stratLine 403 si besoin)
bool request_handler::is_method_allowed(void)
{/* PROBLEME (A TESTER) */
	if (_hrx["A"][1].find(".bla") != string::npos && _hrx["A"][0] == "POST")  // A VIRER SPARADRAP POUR TESTER PROBLEM !!
		return _c->rqst_t = 2 , true;  // A VIRER SPARADRAP POUR TESTER PROBLEM !!
#ifdef _debug_
	cout << BLUE "IS_METHOD_ALLOWED \n" RESET;
#endif
	bool allowed = false;
	const char *array[] = {"GET", "PUT", "POST", "HEAD", "DELETE", NULL};
	for (int i = 0; array[i]; ++i)
		// cout << MAGENTA << *strs << RESET << endl;
		if (array[i] == _hrx["A"][0]) {
			_c->rqst_t = i;
#ifdef _debug_
			cout << GREEN << "_c->rqst_t : " RESET << (int)_c->rqst_t << endl;
#endif
			allowed = true;
		}
	if (!allowed){
		gen_startLine( _status.find("400") );
#ifdef _debug_
		cout << MAGENTA << "400 Bad Request" << RESET << endl;
#endif
		_c->rqst_t = INVALID;
		return allowed;
	}
	allowed = false;
	for (size_t i = 0; i < _si[_s_id].location[_l_id].allowed_method.size(); ++i)
		if (_si[_s_id].location[_l_id].allowed_method[i] == _hrx["A"][0])
			allowed = true;
	if (!allowed)
		gen_startLine( _status.find("405") );
#ifdef _debug_
	cout << MAGENTA << (allowed ? "oui\n" : "non\n") << RESET;
#endif
	return allowed;
}

// Check si le fichier fourni par la rqst est plus long que "max_file_size" defini ds la conf
// maj de la stratLine au cas où : 413 Request Entity Too Large, ds ce cas return npos
size_t request_handler::check_file_size(void)
{
	// EN CAS DE BODY PLUS LONG QU'AUTORISÉ -> 413 (REQUEST ENTITY TOO LARGE)
#ifdef _debug_
	cout << BLUE "DS CHECK_FILE_SIZE()" RESET << endl;
#endif

	if (_c->rqst_transfer_t == CHUNCK)
		if (!_si[_s_id].location[_l_id].max_file_size.empty() && (_c->clen > static_cast<size_t>(atoi(_si[_s_id].location[_l_id].max_file_size.c_str())))) {
			gen_startLine( _status.find("413") );
#ifdef _debug_
	cout << RED "DS CHECK_FILE_SIZE, TROP GROS _c->clen : " RESET << _c->clen << " / conf : " <<  atoi(_si[_s_id].location[_l_id].max_file_size.c_str()) << endl;
#endif			
			return string::npos;
		}

	if (_hrx.find("Content-Length:") == _hrx.end() || _hrx["Content-Length:"].empty())
		return string::npos;
	size_t max_file_size = atoi(_si[_s_id].max_file_size.c_str());
	size_t actual_file_size = atoi(_hrx["Content-Length:"][0].c_str());
	if (!_si[_s_id].max_file_size.empty() && actual_file_size > max_file_size ) {
		gen_startLine( _status.find("413") ); 
#ifdef _debug_
	cout << RED "DS CHECK_FILE_SIZE, TROP GROS actual_file_size : " RESET << actual_file_size << " / conf : " <<  max_file_size << endl;
#endif			

		return string::npos;
	}
#ifdef _debug_
	cout << actual_file_size << endl;
#endif
	return actual_file_size;
}

// Retourne la taille du fichier INUTILISÉ
string request_handler::get_file_size(string& path)
{
	std::ifstream f(path.c_str());
	std::stringstream ss;
	if (!f.is_open()) {
		cout << RED "PAS OPEN!!\n" RESET;
		ss << "-1";
	}
	else {
		f.seekg(0, f.end);
		ss << f.tellg();
		cout << "size : " << f.tellg() << endl;
		f.close();
	}
	return ss.str();
}

// Remove multiple '/' and the '/' at url's end
void request_handler::clean_url(string& str)
{
	for (size_t i = 0; i < str.size(); ++i)
		while (str[i] == '/' && (str[i + 1] == '/' || str[i + 1] == '\0') && str.size() > 1)
			str.erase(i, 1);
}

	/* FUNCTION DE DEBUG */

void request_handler::display(void) 
{
	cout << GREEN ITALIC UNDERLINE "DISPLAY HEADER INFORMATION" RESET GREEN " :" RESET << endl;
	for (map<string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it) {
		cout << it->first << " ";
		for (size_t i = 0; i < it->second.size(); ++i)
			cout << it->second[i] << ", ";
		cout << endl;
	}
}

// Ajoute _PUT ou _POST en fonction de la methode employée et check si le fichier de ce nom existe
// Maj de la startLine en foncion respectibement 204 / 201
void request_handler::does_file_exist(e_rqst_type type) 
{
	// SI LE FICHIER EXISTE NGINX REPOND : "HTTP/1.1 204 No Content", sinon "... 201 Created"
	struct stat sb;
	_c->post_file_path = _path + (type == PUT ? "_PUT" : "_POST");
	if (stat(_c->post_file_path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
		gen_startLine(  _status.find("204") );
	else
		gen_startLine(  _status.find("201") );
}

// Creer et/ou ecrit ds le fichier, (!! ENLEVÉ !!: ainsi que ds _body pour la reponse)
void request_handler::write_file(void) 
{
	ofstream my_file(_c->post_file_path.c_str(), std::ofstream::out | std::ofstream::trunc);
	if (my_file.is_open()) {
		my_file.write(_body.c_str(), _body.size());
		my_file.close();
#ifdef _debug_
		cout << GREEN "FILE WRITTEN !! Upload location : " + _c->post_file_path  + RESET "\n\n";
	if (_c->rqst.size() < 1000)
		cout << _c->rqst + "\n\n" ;
#endif
	}
#ifdef _debug_
	else
		cout << RED "download path invalid : " RESET << _c->post_file_path << "\n";
#endif
}

// Injecte le _body de la requete du client ds les cgi
int request_handler::cgi_input()
{
	cout << BLUE "IN CGI_INPUT()\n" RESET;
	cout << "body size : " << _body.size() << "\n";
	// cout << "body : \n[" << _body << "]\n";

// ::
	while (_c->cgi_byte_write < _body.size()){
		_c->cgi_byte_write += write(_c->cgi_fd[1], &_body[_c->cgi_byte_write], _body.size());
		cout << "cgi_byte_write : [" << _c->cgi_byte_write << "]\n";
	}
	if (_c->cgi_byte_write >= _body.size())
		if (!close(_c->cgi_fd[1]))
			cout << GREEN "_c->cgi_fd[1] closed\n" RESET;

	// cout << "100 1st char of _body in cgi_input : " << _body.substr(0, 100) << endl;
// ::

#ifdef _debug_
	// cout << GREEN "written : " RESET << written << endl;
#endif
	return 0;
}

// Extrait la réponse des cgi vers la réponse à faire au client
int request_handler::cgi_output()
{
#ifdef _debug_
	cout << BLUE "IN CGI_OUTPUT()\n" RESET;
#endif

	char str[100000];
	int len; //, nb = 0;
	_body.clear();
	while ((len = read(_c->cgi_fd[0], str, 100000)) > 0) {// Peut-etre pas faire de while là
		_body.append(str, len);
#ifdef _debug_
		// cout << GREEN "read len : " RESET "[" << len << "]" << " nb [" << ++nb << "]" << endl;
#endif
	}
#ifdef _debug_
	if (_body.size() < 1000)
		cout << GREEN "DS CGI_OUTPUT _body : \n" RESET "[" << _body + "]" << endl;
	else 
		cout << GREEN "DS CGI_OUTPUT _body(TOO_LONG) : \n" RESET "[" << _body.size() << "]" << endl;
#endif
	if (len < 100000) {
		std::cout << "CGI EOF" << std::endl;
		waitpid(-1, NULL, 0);
		clean_body();
// ECRIT EFFECTIVEMENT LA RÉPONSE COMPLETE (AV LES HEADERS)
		gen_startLine( _status.find("200") );
		gen_CLength(3);
		add_all_field(); 
		add_body(true);
		_hrx.clear();
		_htx.clear();

#ifdef _log_
	ofstream _LOGfile;
	_LOGfile.open("POST.txt", std::ofstream::trunc);
	if (_LOGfile.is_open()) {
		_LOGfile << _c->resp;
		_LOGfile.close();
	}
#endif
	}
	return 0;
}


  std::string to_upper(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
  }

char** request_handler::setCGIEnv()
{
#ifdef _debug_
	cout << BLUE "IN SETCGIENV() : " RESET << (int)_c->rqst_t << "\n";
#endif

	std::map< std::string, std::string > cgi_env_;

  if (_c->rqst_t == POST){
		cgi_env_["CONTENT_TYPE"] = _hrx["Content-Type:"].empty() ? "" : _hrx["Content-Type:"][0];// req_headers_["Content-Type"];
		cgi_env_["CONTENT_LENGTH"] = _hrx["Content-Length:"].empty() ? patch::to_string(_body.size()) : _hrx["Content-Length:"][0] ; // ft::to_string(req_body_.length());
	}
	cgi_env_["GATEWAY_INTERFACE"] = "CGI/1.1";
  cgi_env_["PATH_INFO"] = _path; //"ATwebserv/YoupirBanane"; // file_path_;
	cgi_env_["PATH_TRANSLATED"] = _path; // "ATwebserv/YoupirBanane"; // file_path_;
  cgi_env_["QUERY_STRING"] = _URLquery;//config_.getQuery();
  cgi_env_["REMOTE_ADDR"] = "127.0.0.1"; //config_.getClient().getAddr();

/* 
//   if (config_.getAuth() != "off") {
  if (_hrx.count("X-Secret-Header-For-Test:")) {
	cgi_env_["AUTH_TYPE"] = "Basic";
	// cgi_env_["REMOTE_IDENT"] = config_.getAuth().substr(0, config_.getAuth().find(':'));
	cgi_env_["REMOTE_IDENT"] = "test";
	// cgi_env_["REMOTE_USER"] = config_.getAuth().substr(0, config_.getAuth().find(':'));
	cgi_env_["REMOTE_USER"] = "test";

  }
 */

  cgi_env_["REQUEST_METHOD"] = _hrx["A"][0]; //config_.getMethod();
	cgi_env_["REQUEST_URI"] = _path; // file_path_;

  cgi_env_["SCRIPT_NAME"] = _si[_s_id].location[_l_id].cgi_path; // cgi_path_;
	cgi_env_["SERVER_NAME"] = _si[_s_id].server_name; // "WEBSERV/1.0"; //config_.getHost();
	cgi_env_["SERVER_PROTOCOL"] = "HTTP/1.1"; // config_.getProtocol();
	cgi_env_["SERVER_PORT"] = _si[_s_id].port; // "8080"; // ft::to_string(config_.getPort());
  cgi_env_["SERVER_SOFTWARE"] = "WEBSERV/1.0";

//	if (extension_ == ".php")
//		cgi_env_["REDIRECT_STATUS"] = "200";

  for (std::map<std::string, std::vector <std::string> >::iterator it = _hrx.begin(); it != _hrx.end(); it++) {
	if (!it->second.empty()) 
		if (it->first == "X-Secret-Header-For-Test:")	{
	  std::string header = "HTTP_" + to_upper(it->first);
	//   header.pop_back();
	  header.erase(header.size() - 1);
	  std::replace(header.begin(), header.end(), '-', '_');
	  cgi_env_[header] = it->second[0];
	}
  }
		char **env_;
	if (!(env_ = (char **)malloc(sizeof(char *) * (cgi_env_.size() + 1))))
		return NULL;

	int i = 0;

	for (std::map<std::string, std::string>::iterator it = cgi_env_.begin(); it != cgi_env_.end(); it++) {
		std::string tmp = it->first + "=" + it->second;
		if (!(env_[i] = strdup(tmp.c_str())))
					return NULL;
#ifdef _debug_
		else
			std::cout << "env : " << env_[i] << std::endl;
#endif
		i++;
	}
	env_[i] = NULL;
	return env_;
}


// Fait trop de truc...
int	request_handler::handle_cgi(void)
{
#ifdef _debug_
	cout << BLUE "IN HANDLE_CGI()\n" RESET;
#endif
	
	if (_si[_s_id].location[_l_id].cgi_path.empty())
	{
#ifdef _debug_
		cout << YELLOW "cgi_path.empty\n" RESET;
#endif
		gen_startLine( _status.find("403") );
		return 1;
	}
		// SI TOUT S'EST BIEN PASSÉ ON DÉTACHE LE BODY DE LA REQUETE POST (DS _BODY)
	if (_c->rqst_transfer_t == MULTIPART)
		extract_multi_rqst_body();
	else if (_c->rqst_transfer_t == URL_ENCODED) // Content-Type: application/x-www-form-urlencoded
		extract_xform_rqst_body();
	else if (_c->rqst_transfer_t == CHUNCK)
		extract_chunk_rqst_body();


	if (launch_cgi()){

		if (cgi_input()) // Ne peut pas renvoyer autre chose que 0 pour le moment
			return 1;

#ifdef _debug_
	cout << YELLOW "3\n" RESET;
	if (_body.size() < 1000)
		std::cout << "Buffer copied in body :\n[" << _body + "]\n";
	else
		std::cout << "Buffer copied in body(TOO_LONG) :[" <<  _body.size() << "]\n";
#endif
		cgi_output();
	}
	else 
		return 1;

	return (0);
}

// Détecte s'il s'agit d'un cgi
bool	request_handler::is_cgi()
{
#ifdef _debug_
	cout << BLUE "IN IS_CGI() : " RESET;
#endif

	size_t	i;
	string query; // SI l'url comporte des argument après '?' on la tronque
	query = ((i = _path.find('?')) != string::npos) ? _path.substr(0, i) : _path;
	if (i != string::npos) {
		_URLquery = _path.substr(i + 1);
		_path = _path.substr(0 , i);
#ifdef _debug_
		cout << GREEN "_path : " RESET << _path << GREEN "_URLquery : " RESET<< _URLquery << "\n" ;
#endif
	}
	query = ((i = query.find_last_of('.')) != string::npos) ? query.substr(i) : query;
	i = 0;
	std::vector<std::string>& extensions = _si[_s_id].location[_l_id].cgi_file_types;
	for (std::vector<std::string>::iterator type = extensions.begin(); type != extensions.end(); type++, ++i)
	{
		if ((query.find(*type)) != std::string::npos){
#ifdef _debug_
			cout << "cgi type -> " + *type << endl;
#endif
			_c->ext_id = i;
			return (true);
		}
	}
#ifdef _debug_
	cout << "nop_not_cgi" << endl;
#endif
	return (false);
}

// Lance le fork() cgi avec le _path du fichier, ouvre les _c->cgi_fd[2];
int	request_handler::launch_cgi()
{
#ifdef _debug_
	cout << BLUE "IN LAUNCH_CGI()\n" RESET;
	cout << "path : " << _path << endl;
	cout << "cgi_program : " << _si[_s_id].location[_l_id].cgi_path.c_str() << endl;
#endif

	pid_t		pid;
	int			input_fd[2];	// pipe du body des requetes vers l'executable cgi
	int			output_fd[2];	// pipe de l'executable cgi vers les réponses des requetes
	char		*e_path[4] = {	(char*)_si[_s_id].location[_l_id].cgi_path.c_str(),
								(char*)_path.c_str(),
								// (char*)"-c",
								// (char*)"files/scripts/php.ini",
								NULL
							 };

	if (pipe(input_fd) == -1 || pipe(output_fd) == -1) {
		// throw (std::runtime_error("Pipe initialization failled"));
		gen_startLine( _status.find("500") );
		return 0;	
	}
	// fcntl(input_fd[0], F_SETFL, O_NONBLOCK); // <- A REMMETTRE ?
	// fcntl(input_fd[1], F_SETFL, O_NONBLOCK); // <- A REMMETTRE ?
	// fcntl(output_fd[0], F_SETFL, O_NONBLOCK); // <- A REMMETTRE ?

	// std::ifstream t(_path);
	// if (!t.is_open()) {
	// 	gen_startLine( _status.find("404") );
	// 	cout << RED "PAS OPEN!!\n";
	// 	return 0;
	// }

	pid = fork();
	if (pid == -1) {
		// throw (std::runtime_error("fork initialization failled"));
		gen_startLine( _status.find("500") );
		return 0;	
	}
	if (pid == 0)
	{ // DS L'ENFANT
#ifdef _debug_
		cout << GREEN "\nCGI_FORK OK !\n" RESET;
#endif

		char**c_env = setCGIEnv();

		close(input_fd[1]);
		close(output_fd[0]);
		dup2(input_fd[0], STDIN_FILENO);	 // read
		dup2(output_fd[1], STDOUT_FILENO);	// write

		execve(*e_path, e_path, c_env);
		exit(1);
	}
	else // DS LE PARENT (on close les extrémités des pipes non utilisés)
	{
		close(input_fd[0]); // read
		close(output_fd[1]); // write
	}
	_c->cgi_fd[0] = output_fd[0];// ici on lit depuis l'executable des cgi
	_c->cgi_fd[1] = input_fd[1]; // ici on ecrit vers l'executable des cgi
	return (1);
}

// On ne sait pas ce que ça fait
void	request_handler::clean_body()
{
#ifdef _debug_
	cout << BLUE "IN CLEAN_BODY()\n" RESET;
#endif

	std::string	tmp;
	std::string	index;
	size_t	pos;
	
	while ((pos = _body.find("\r\n")) != std::string::npos)
	{
		tmp = _body.substr(0, pos + 2);
		_body = _body.substr(pos + 2);
		if (tmp == "\r\n")
			break ;
		if ((pos = tmp.find(":")) != std::string::npos)
		{
			index = tmp.substr(0, pos);
#ifdef _debug_
			cout << GREEN "IN CLEAN_BODY index=" RESET << index << endl;
#endif
			if (index != "X-Powered-By")
				_htx[index].push_back(tmp.substr(0, tmp.find("\r\n") + 2));
		}
	}
}
