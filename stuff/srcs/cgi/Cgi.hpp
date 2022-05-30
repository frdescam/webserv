#ifndef CGI_HPP
# define CGI_HPP

# include "./../../inc/webserv.hpp"
# include "../request/Request.hpp"
# include "../config/Config.hpp"

class Cgi
{
	public:
		Cgi(void);
		~Cgi(void);
		Cgi(Cgi const &other);
		Cgi(std::string &path, bool is_post, std::string &infile, std::map<std::string, std::string> &env_vars);
		Cgi		&operator=(const Cgi &other);
		void	execute(void);

	private:
		bool								_post;
		std::string							_path_to_cgi;
		std::string							_infile;
		std::map<std::string, std::string>	_env_vars;
};

#endif
