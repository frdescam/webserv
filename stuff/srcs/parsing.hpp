#ifndef PARSER_HPP
# define PARSER_HPP

# include "webserv.hpp"
# include "config_handler.hpp"

class parsing
{
	public:
		parsing(void);
		~parsing(void);
		parsing(parsing const &other);
		parsing(std::map<std::string, std::string> &env_var, config_handler &block);
		parsing & operator=(parsing const &other);
		std::map<std::string, std::string>	parseOutputclient(std::string &output);
		bool	isPost(void);
		bool	isChunked(void);
		size_t	getLengthBody(void);
		size_t	getLengthHeader(void);
		config_handler	getBlock(void);
		int		getFlag(void);

	private:
		bool								_post;
		bool								_chunked;
		int									_flag;
		std::size_t							_length_body;
		std::size_t							_length_header;
		std::string							_header;
		std::string							_method;
		std::string							_content_length;
		std::string							_content_type;
		std::map<std::string, std::string>	_env_vars;
		config_handler								_block;

		void		_parseQueryString(std::string & request_uri);
		void		_parserequestMethod(std::string & output, std::size_t & pos);
		void 		_parserequestUri(std::string & output, std::size_t & pos);
		void		_parseserverProtocol(std::string & output, std::size_t & pos);
		void		_parseserverPort(std::string & output, std::size_t & pos);
		void		_parseContentLength(std::string & output);
		void 		_parseContentType (std::string & output);
		void 		_parseHttpAccept(std::string &output, std::string tofind);
		void		_parseTransferEncoding(std::string & output);
		void		_parseScript(std::string & request_uri);
		void 		_addIndex(void);
		void		_changeBlockToNewconfig_handler(config_handler &newconfig_handler);
		void 		_chooseconfig_handlerBeforeExecution(void);
		std::string	_getLocationBeforeExecution(std::string path, config_handler &tmpBlock, config_handler &newconfig_handler);
};

#endif
