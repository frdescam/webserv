#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "config_handler.hpp"
#include "cgi.hpp"
#include "response.hpp"
#include "parsing.hpp"
#include "webserv.hpp"

class request
{
	public:
		request(void);
		~request(void);
		request(const request &other);
		request(std::string &request_str, int rc, config_handler &block, int id);
		request	&operator=(const request &other);

		bool	isComplete(void);
		bool	isPost(void);
		bool	isChunked(void);
		bool	sentContinue(void);
		void	addToBody(const std::string &request_str, int pos, int len);
		void	addToBodyChunked(const std::string &request_str, int len);
		int		getFd(void);
		FILE	*getFp(void);
		void	setFpToNull(void);
		int		writeInFile(void);

		response									executeChunked(void);
		response									execute(void);
		config_handler										&getConf(void);
		int											getFlag(void);


	private:
		config_handler								_block;
		std::string							_path_to_cgi;
		std::string							_tmp_file;
		bool								_completed;
		bool								_cgi;
		bool								_chunked;
		bool								_post;
		bool								_header_completed;
		bool								_sent_continue;
		bool								_last_chunk_received;
		size_t								_body_part_len;
		size_t								_length_body;
		size_t								_length_header;
		size_t								_length_received;
		size_t								_length_of_chunk;
		int									_fd;
		int									_flag;
		std::string							_body_part;
		std::map<std::string, std::string>	_env_vars;
		FILE				*				_fp;

		void		_initEnvMap(void);
		void 		_initPostrequest(const std::string &request_str, int rc, int id);
		void 		_addToLengthReceived(size_t length_to_add);
		void		_checkLastBlock(void);
		response	_executeGet(response &r);
		response	_executePost(response &r);
		response	_executeDelete(response &r);
		response	_executeRedirection(response &r);
};

#endif
