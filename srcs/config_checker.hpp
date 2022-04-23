#ifndef _CONFIG_CHECKER_HPP_
#define _CONFIG_CHECKER_HPP_
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <map>
#include "color.hpp"
#include "struct_webserv.hpp"

using namespace std;

class config_checker
{

/* 
**		TYPES
*/
	typedef std::vector<server_info>*		info_vector;

	/* data */
	class configException: public std::exception
	{
	  public :
	  configException(info_vector si): _si(si), _str("Config File : error.") {(void)_si;};
	  configException(info_vector si, std::string str) : _si(si), _str(str + "\nConfig File : error.") {(void)_si;};
	  ~configException() throw() {};
	  virtual const char* what() const throw()
	  {
		// delete _si;
	    return  _str.c_str();
	  }
	  private :
	  info_vector _si;
	  std::string _str;
	};

/* 
**		ATTRIBUTS
*/
public:
	info_vector _si;
// ↓ obsolete ajd, etait prevu pour charger le fichier de config reference au cas oú on faisait un checker costaud
	map<string, vector<string> > _semantic; 
	

	config_checker(/* args */);
	~config_checker();

// le throw spécifier doit throw le type exact sinon std::unexpected is called and the catch block doesn't work # https://openclassrooms.com/fr/courses/1894236-programmez-avec-le-langage-c/1903837-gerez-des-erreurs-avec-les-exceptions#/id/r-1903804
	void check_conFile(std::string); // throw (/* Si on mets rien ça throw rien */) {throw (configException());}; 

private:
/* 
**		PRIVATE FUNCTION (UTILITAIRES)
*/
	void check_serv_part(std::ifstream&, server_info&);
	void check_loca_part(std::ifstream&, server_info&);
	// bool is_not_allowed(string key,string val);
	// void string_vector_insert(ifstream& ifs, string& where_to_insert);
	void valid_port(std::ifstream& ifs, server_info&);
	// void valid_server_nm(std::ifstream& ifs);
	// void valid_error_page(std::ifstream& ifs);
	void extract_to_vector(std::ifstream& ifs, std::vector<string>& v);
	void extract_to_string(std::ifstream& ifs, string& s);
};

#endif
