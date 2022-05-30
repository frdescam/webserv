#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "webserv.hpp"

class config_handler
{
	public:
		config_handler(void);
		~config_handler(void);
		config_handler(config_handler const &other);
		config_handler & operator=(config_handler const &other);

		std::string								&getIpAddress(void);
		int										&getPort(void);
		std::vector<std::string>				&getserverNames(void);
		std::map<int, std::string>				&getErrorPages(void);
		unsigned long long						&getclientMaxBodySize(void);
		std::string 							&getcgiPass(void);
		std::vector<std::string>				&getAlowMethods(void);
		std::map<std::string, config_handler>			&getLocation(void);
		std::string								&getRoot(void);
		std::vector<std::string>				&getIndex(void);
		bool									&getAutoIndex(void);
		std::string								&getUploadFolder(void);
		std::pair<std::string, std::string>		&getRedirection(void);
		int										parseserver(std::vector<std::vector<std::string> > confFile, size_t i);
		void									checkBlock(bool location);

	private:
		std::string								_ipAddress;
		int										_port;
		std::vector<std::string>				_serverNames;
		std::map<int, std::string>				_errorPages;
		unsigned long long						_clientMaxBodySize;
		std::string								_cgiPass;
		std::vector<std::string>				_allowMethods;
		std::map<std::string, config_handler>			_location;
		std::string								_root;
		std::vector<std::string>				_index;
		bool									_autoIndex;
		std::string								_uploadFolder;
		std::pair<std::string, std::string>		_redirection;

		// Parse file .conf
		int		_parseLocationDeep(std::vector<std::vector<std::string> > confFile, size_t i);

		// SET
		void	_setListen(std::vector<std::string> line);
		void	_setserverName(std::vector<std::string> line);
		void	_setErrorPage(std::vector<std::string> line);
		void	_setclientMaxBodySize(std::vector<std::string> line);
		void	_setcgiPass(std::vector<std::string> line);
		void	_setAllowMethods(std::vector<std::string> line);
		int		_setLocation(std::vector<std::vector<std::string> > confFile, size_t i);
		void	_setRoot(std::vector<std::string> line);
		void	_setIndex(std::vector<std::string> line);
		void	_setAutoIndex(std::vector<std::string> line);
		void	_setUploadFolder(std::vector<std::string> line);
		void	_setRedirection(std::vector<std::string> line);
		void	_removeLastSlashe(std::string &path);
};

std::ostream	&operator<<(std::ostream &out, config_handler &conf);

#endif
