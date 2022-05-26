#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "./../../inc/webserv.hpp"
# include <dirent.h>

class Response
{

	public:
		Response(void);
		~Response(void);
		Response(const Response &other);
		Response	&operator=(const Response &other);

		std::string	&getRawResponse(void);
		std::map<int, std::string>	&getErrorPages(void);
		void	setErrorPages(const std::map<int, std::string> &new_errorPages);
		void	createCgiGet(const std::string &filename);
		void	createCgiPost(const std::string &filename, const std::string &upload_path);
		void	createGet(const std::string &filename);
		void	createContinue(void);
		void	createRedirection(const std::string &redirection);
		void	createDelete(const std::string &filename);
		void	printDirectory(const std::string &root_dir, const std::string &dir);
		void	error(const std::string &error_code);
		void	addToLengthSent(size_t block_size);
		bool	isEverythingSent(void);
		void	setLengthResponseSizeT(size_t len_of_string);
		size_t	getRemainingLength(void);
		size_t	getLengthSent(void);
		void	reset(void);

	private:
		std::string							_header;
		std::string							_body;
		std::string							_raw_response;
		std::string							_filename;
		bool								_sent_all;
		bool								_is_binary;
		size_t								_length_sent;
		size_t								_length_response;
		std::map<std::string, std::string>	_mimes;
		std::map<int, std::string>			_errorPages;


		void			_settingMimes(void);
		std::string		_getErrorMessage(const std::string &error_code);
		void			_createCgi(const std::string &filename, const std::string &header);
		std::string 	_getPathToError(const std::string &error_code);
		void			_binary(const std::string &filename);
		std::streampos	_lengthOfFile(std::ifstream &f);
};

#endif
