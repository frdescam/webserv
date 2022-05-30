#include "response.hpp"

response::response(void): _sent_all(false), _is_binary(false),
	_length_sent(0), _length_response(0)
{
	this->_settingMimes();
}

response::~response(void)
{}

response::response(const response &other): _header(other._header), _body(other._body), _raw_response(other._raw_response), _filename(other._filename),
	_sent_all(other._sent_all), _is_binary(other._is_binary), _length_sent(other._length_sent), _length_response(other._length_response)
{
	this->_errorPages = other._errorPages;
	this->_mimes = other._mimes;
}

response	&response::operator=(const response &other)
{
	if (this != &other)
	{
		this->_body = other._body;
		this->_header = other._header;
		this->_raw_response = other._raw_response;
		this->_filename = other._filename;
		this->_sent_all = other._sent_all;
		this->_is_binary = other._is_binary;
		this->_length_sent = other._length_sent;
		this->_length_response = other._length_response;
		this->_errorPages = other._errorPages;
		this->_mimes = other._mimes;
	}
	return (*this);
}

void	response::reset(void)
{
	this->_header.clear();
	this->_body.clear();
	this->_raw_response.clear();
	this->_filename.clear();
	this->_sent_all = false;
	this->_is_binary = false;
	this->_length_response = 0;
	this->_length_sent = 0;
}

std::string					&response::getRawresponse(void) {return this->_raw_response;}
bool						response::isEverythingSent(void) {return this->_sent_all;}
size_t						response::getRemainingLength(void) {return this->_length_response - this->_length_sent;}
size_t						response::getLengthSent(void) {return this->_length_sent;}
std::map<int, std::string>	&response::getErrorPages(void) {return this->_errorPages;}

void	response::addToLengthSent(size_t block_size)
{
	this->_length_sent += block_size;
	if (this->_length_sent == this->_length_response)
		this->_sent_all = true;
}

void	response::setLengthresponseSizeT(size_t len_of_string)
{
	this->_length_response = len_of_string;
}

void	response::setErrorPages(const std::map<int, std::string> &new_errorPages)
{
	this->_errorPages = new_errorPages;
}

void	response::_createcgi(const std::string &filename, const std::string &begin_header)
{
	std::ifstream		f(filename.c_str());
	std::stringstream	ss;
	std::streampos		len_of_file;
	std::string			str, body, header(begin_header);
	bool				has_header = false;

	this->_filename = filename;
	f.clear();
	f.seekg(0, std::ios::beg);
	if (f)
	{
		while (f.good())
		{
			str.clear();
			getline(f, str);
			if (!has_header && (str.empty() || str.size() <= 1))
			{
				has_header = true;
				continue ;
			}
			if (!has_header)
			{
				header.append(str + "\n");
			}
			else
			{
				body.append(str);
				body.append("\r\n");
			}
		}
	}
	f.close();
	ss << body.size();
	header.append("Content-Length: ");
	header.append(ss.str());
	header.append("\r\n\r\n");
	this->_raw_response.append(header);
	this->_raw_response.append(body);
	this->setLengthresponseSizeT(this->_raw_response.size());
}

void	response::createcgiPost(const std::string &filename, const std::string &upload_path)
{
	std::string			header;
	std::ifstream		f(filename.c_str());
	std::stringstream	buffer;
	std::string 		uploaded_file, str;

	f.clear();
	f.seekg(0, std::ios::beg);
	if (f)
	{
		buffer << f.rdbuf();
		str = buffer.str();
		header = "HTTP/1.1 201 Created\r\nConnection: keep-alive\r\n";
		if (str.find("Success") != std::string::npos)
		{
			std::size_t i = str.find("<br>") + 4;
			if (i != std::string::npos)
			{
				size_t	length = str.find(" ", i);
				uploaded_file = str.substr(i, length - i);
				header.append("Location: " + upload_path + "/" + uploaded_file + "\r\n");
			}
		}
		else if (str.find("Error") != std::string::npos)
			header = "HTTP/1.1 409 Conflict\r\nConnection: keep-alive\r\n";
	}
	else
	{
		f.close();
		return this->error("500");
	}
	f.close();
	this->_createcgi(filename, header);
}

void	response::createcgiGet(const std::string &filename)
{
	std::string		header = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\n";
	this->_createcgi(filename, header);
}

void	response::createContinue(void)
{
	std::string		header("HTTP/1.1 100 Continue\r\n\r\n");
	this->_raw_response = header;
	this->setLengthresponseSizeT(this->_raw_response.size());
}

std::streampos	response::_lengthOfFile(std::ifstream &f)
{
	std::streampos	fsize = f.tellg();
	f.seekg(0, std::ios::end);
	fsize = f.tellg() - fsize;
	f.seekg(0, std::ios::beg);
	return fsize;
}

void	response::createGet(const std::string &filename)
{
	this->_filename = filename;
	if ((filename.size() <= 5 || filename.compare(filename.size() - 5, 5, ".html") != 0) && (filename.size() <= 4 || filename.compare(filename.size() - 4, 4, ".txt") != 0))
	{
		this->_binary(filename);
		return ;
	}
	std::ifstream 		f(filename.c_str());
	std::stringstream	ss;
	std::string			header("HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: ");
	std::string			str, body;
	if (f)
	{
		while (f.good())
		{
			getline(f, str);
			body.append(str);
			body.append("\r\n");
		}
	}
	else
		return this->error("500");
	f.close();
	ss << body.size();
	header.append(ss.str());
	header.append("\r\n\r\n");
	this->_raw_response.append(header);
	this->_raw_response.append(body);
	this->setLengthresponseSizeT(this->_raw_response.size());
}

void	response::createRedirection(const std::string &redirection)
{
	std::string			header("HTTP/1.1 301 Moved Permanently\r\nLocation: " +
		redirection + "\r\nConnection: keep-alive\r\nContent-Length: 0");

	header.append("\r\n\r\n");
	this->_raw_response.append(header);
	this->setLengthresponseSizeT(this->_raw_response.size());
}

void	response::_binary(const std::string &filename)
{
	std::size_t			found, length;
	std::string			header, extension;
	std::stringstream	ss;
	std::ifstream		f(filename.c_str(), std::ios::binary);

	this->_is_binary = true;
	header = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: *\r\nContent-Length: ";
	found = filename.find_last_of(".");
	extension = filename.substr(found);
	std::map<std::string, std::string>::iterator it = this->_mimes.find(extension);

	if (it != this->_mimes.end())
		header.replace(header.find("*"), 1, it->second);
	else
		header.replace(header.find("*"), 1, "application/octet-stream");
	if (!f)
		return this->error("500");

	f.clear();
	f.seekg(0, std::ios::end);
	length = f.tellg();
	f.seekg(0, std::ios::beg);

	ss << length;
	header.append(ss.str());
	header.append("\r\n\r\n");
	this->_raw_response.append(header);
	std::string	content((std::istreambuf_iterator<char>(f)), (std::istreambuf_iterator<char>()));
	f.close();
	this->_raw_response.append(content);
	this->setLengthresponseSizeT(this->_raw_response.size());
}

void	response::_settingMimes(void)
{
	this->_mimes[".avi"] = "video/x-msvideo";
	this->_mimes[".bmp"] = "image/bmp";
	this->_mimes[".csv"] = "text/csv";
	this->_mimes[".epub"] = "application/epub+zib";
	this->_mimes[".gif"] = "image/gif";
	this->_mimes[".html"] = "text/html";
	this->_mimes[".htm"] = "text/html";
	this->_mimes[".php"] = "text/plain";
	this->_mimes[".js"] = "text/plain";
	this->_mimes[".css"] = "text/css";
	this->_mimes[".jpg"] = "image/jpg";
	this->_mimes[".jpeg"] = "image/jpeg";
	this->_mimes[".json"] = "application/json";
	this->_mimes[".mpeg"] = "video/mpeg";
	this->_mimes[".png"] = "image/png";
	this->_mimes[".pdf"] = "application/pdf";
	this->_mimes[".svg"] = "image/svg+xml";
	this->_mimes[".xml"] = "application/xml";
	this->_mimes[".zip"] = "application/zip";
	this->_mimes[".*"] = "application/octet-stream";
}

std::string	response::_getErrorMessage(const std::string &error_code)
{
	std::map<std::string, std::string> error_map;
	error_map["400"] = "Bad request";
	error_map["403"] = "Forbidden";
	error_map["404"] = "Not Found";
	error_map["405"] = "Method Not Allowed";
	error_map["413"] = "Payload too large";
	error_map["500"] = "Internal server Error";
	error_map["200"] = "OK";
	error_map["201"] = "Created";
	error_map["409"] = "Conflict";
	return (error_map[error_code]);
}

void	response::error(const std::string &error_code)
{
	std::string			error_page(this->_getPathToError(error_code)), error_message(this->_getErrorMessage(error_code));
	std::ifstream		f(error_page.c_str());
	std::stringstream	ss;
	std::string			header("HTTP/1.1 " + error_code +" "+error_message+"\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nContent-Length: ");
	std::string         str, body;

	this->_filename = error_page;
	if (!error_code.compare("413"))
		header.replace(header.find("keep-alive"), 10, "close");
	if (f)
	{
		while (f.good())
		{
			getline(f, str);
			body.append(str);
			body.append("\r\n");
		}
	}
	else if (error_code.compare("500"))
		return this->error("500");
	else
		throw("Error 500");
	f.close();
	ss << body.size();
	header.append(ss.str());
	header.append("\r\n\r\n");
	this->_raw_response.append(header);
	this->_raw_response.append(body);
	this->setLengthresponseSizeT(this->_raw_response.size());
}

void	response::printDirectory(const std::string &root_dir, const std::string &dir)
{
	DIR					*dpdf;
	struct dirent		*epdf;
	std::stringstream	ss;
	std::string			header("HTTP/1.1 200 OK\r\nConnection: keep-alive\r\n");
	std::string			str, body;
	struct stat			buf;

	dpdf = opendir(root_dir.c_str());
	if (dpdf != NULL)
	{
		header.append("Content-Length: ");
		body.append("<h1>INDEX</h1>");
		while ((epdf = readdir(dpdf)))
		{
			body.append("<a href=\"");
			body.append(dir);
			body.append(epdf->d_name);
			std::string	is_dir = epdf->d_name;
			is_dir = root_dir + is_dir;
			stat(is_dir.c_str() ,&buf);
			if (S_ISDIR(buf.st_mode) != 0)
				body.append("/");
			body.append("\">");
			body.append(epdf->d_name);
			body.append("</a><br>\r\n");
	  }
	}
	else
		return this->error("500");
	closedir(dpdf);
	ss << body.size();
	header.append(ss.str());
	header.append("\r\n\r\n");
	this->_raw_response.append(header);
	this->_raw_response.append(body);
	this->setLengthresponseSizeT(this->_raw_response.size());
}

void	response::createDelete(const std::string &filename)
{
	std::stringstream	ss;
	std::string			header("HTTP/1.1 200 OK\r\nConnection: keep-alive\r\n");
	std::string			body;

	header.append("Content-Length: ");
	body.append(filename);
	body.append(" deleted.\r\n");

	ss << body.size();
	header.append(ss.str());
	header.append("\r\n\r\n");
	this->_raw_response.append(header);
	this->_raw_response.append(body);
	this->setLengthresponseSizeT(this->_raw_response.size());
}

std::string response::_getPathToError(const std::string &error_code)
{
	std::string	path;

	path = this->_errorPages[atoi(error_code.c_str())];
	if (path.empty())
		path = DEFAULT_ERRORS_PATH + error_code + ".html";
	return (path);
}
