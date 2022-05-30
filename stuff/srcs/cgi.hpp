#ifndef CGI_HPP
# define CGI_HPP

# include "webserv.hpp"
# include "request.hpp"
# include "config_handler.hpp"

class cgi
{
	public:
		cgi(void);
		~cgi(void);
		cgi(cgi const &other);
		cgi(std::string &path, bool is_post, std::string &infile, std::map<std::string, std::string> &env_vars);
		cgi		&operator=(const cgi &other);
		void	execute(void);

	private:
		bool								_post;
		std::string							_path_to_cgi;
		std::string							_infile;
		std::map<std::string, std::string>	_env_vars;
};

#endif
