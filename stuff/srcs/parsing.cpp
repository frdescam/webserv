#include "parsing.hpp"

parsing::parsing(void): _post(false), _chunked(false), _flag(0), _length_body(0), _length_header(0),
	_header(), _method(), _content_length(), _content_type()
{}

parsing::~parsing(void)
{}

parsing::parsing(parsing const &other):
	_post(other._post), _chunked(other._chunked), _flag(other._flag), _length_body(other._length_body), _length_header(other._length_header),
	_header(other._header), _method(other._method), _content_length(other._content_length), _content_type(other._content_type),
	_env_vars(other._env_vars), _block(other._block)
{}

parsing::parsing(std::map<std::string, std::string> &env_vars, config_handler &block): _post(false), _chunked(false), _flag(0), _length_body(0), _length_header(0),
	_header(), _method(), _content_length(), _content_type(), _block(block)
{
	this->_env_vars = env_vars;
}

parsing& parsing::operator=(parsing const &other)
{
	if (this != &other)
	{
		this->_post = other._post;
		this->_chunked = other._chunked;
		this->_length_body = other._length_body;
		this->_length_header = other._length_header;
		this->_header = other._header;
		this->_method = other._method;
		this->_content_length = other._content_length;
		this->_content_type = other._content_type;
		this->_env_vars = other._env_vars;
		this->_block = other._block;
		this->_flag = other._flag;
	}
	return *this;
}

bool	parsing::isPost(void) {return _post;}
bool	parsing::isChunked(void) {return _chunked;}
size_t	parsing::getLengthBody(void) {return this->_length_body;}
size_t	parsing::getLengthHeader(void) {return this->_length_header;}
config_handler	parsing::getBlock(void) {return this->_block;}
int		parsing::getFlag(void) {return this->_flag;}

std::map<std::string, std::string>	parsing::parseOutputclient(std::string &output)
{
	size_t i = 0;

	this->_length_header = output.find("\r\n\r\n");
	if (this->_length_header != std::string::npos)
	{
		this->_header = output.substr(0, output.find("\r\n\r\n"));
		this->_length_header += 4;
	}
	else
	{
		this->_header = output;
		this->_length_header = this->_header.size();
	}
	_parserequestMethod(output, i);
	_parserequestUri(output, i);
	_parseserverProtocol(output, i);
	_parseserverPort(output, i);
	_parseTransferEncoding(output);
	_parseContentType(output);
	_parseHttpAccept(output, "Accept:");
	_parseHttpAccept(output, "Accept-Encoding:");
	_parseHttpAccept(output, "Accept-Language:");

	if (this->_env_vars["DOCUMENT_ROOT"].compare("/") != 0)
		this->_env_vars["SCRIPT_FILENAME"] = this->_env_vars["DOCUMENT_ROOT"] + this->_env_vars["SCRIPT_NAME"];
	else
		this->_env_vars["SCRIPT_FILENAME"] = this->_env_vars["SCRIPT_NAME"];

	this->_env_vars["REDIRECT_STATUS"] = "200";

	if (!this->_method.compare("POST"))
	{
		this->_post = true;
		_parseContentLength(output);
		this->_env_vars["PATH_INFO"] = this->_env_vars["SCRIPT_NAME"];
		this->_env_vars["PATH_TRANSLATED"] = this->_env_vars["DOCUMENT_ROOT"] + this->_env_vars["REQUEST_URI"];
	}
	else
	{
		this->_post = false;
		this->_length_body = 0;
	}
	this->_chooseconfig_handlerBeforeExecution();
	return this->_env_vars;
}

void	parsing::_parseQueryString(std::string &request_uri)
{
	std::size_t	i = 0;

	if ((i = request_uri.find("?")) != std::string::npos)
	{
		if (i < request_uri.length())
			this->_env_vars["QUERY_STRING"] = request_uri.substr(i + 1, request_uri.length() - (i + 1));
	}
}

void	parsing::_parserequestMethod(std::string &output, std::size_t &pos)
{
	std::size_t	i = 0;
	std::string	methods[] = {"GET", "POST", "DELETE"};

	while (i < 3)
	{
		if (output.substr(0, methods[i].length()).compare(methods[i]) == 0) 
		{
			this->_env_vars["REQUEST_METHOD"] = methods[i];
			this->_method = methods[i];
			pos += methods[i].length();
			break ;
		}
		i++;
	}
}

void parsing::_parserequestUri(std::string &output, std::size_t &pos) {

	std::size_t	i = 0, length_uri = 0;
	std::string	request_uri;

	i = output.find("/");
	while (!std::isspace(output.at(i + length_uri)))
		length_uri++;
	request_uri = output.substr(i, length_uri);
	this->_env_vars["REQUEST_URI"] = request_uri;
	pos += (i - pos) + length_uri;
	this->_parseQueryString(request_uri);
	if (!this->_env_vars["QUERY_STRING"].empty())
		request_uri = request_uri.substr(0, request_uri.find("?"));
	this->_parseScript(request_uri);
	if (this->_env_vars["SCRIPT_NAME"].empty() && this->_env_vars["REQUEST_URI"][this->_env_vars["REQUEST_URI"].size() - 1] != '/')
		this->_env_vars["REQUEST_URI"].push_back('/');
}

void	parsing::_parseScript(std::string &request_uri)
{
	std::size_t	i;
	std::string	script;

	if ((i = request_uri.find_last_of(".")) != std::string::npos)
	{
		i += 1;
		script = request_uri.substr(0, i);
		while (std::isalpha(request_uri[i]))
			script.push_back(request_uri[i++]);
		this->_env_vars["SCRIPT_NAME"] = script;
	}
	else
		this->_env_vars["SCRIPT_NAME"] = "";
}

void	parsing::_parseserverProtocol(std::string &output, std::size_t &pos)
{
	std::size_t	i = 0, length_protocol = 0;
	std::string	protocols[2] = {"HTTP", "0"};

	while (protocols[i].compare("0") != 0)
	{
		if ((i = output.find(protocols[i], pos)) != std::string::npos)
		{
			while (!std::isspace(output.at(i + length_protocol)))
				length_protocol++;
			this->_env_vars["SERVER_PROTOCOL"] = output.substr(i, length_protocol);
			pos += (i - pos) + length_protocol + 8;
			break ;
		}
		i++;
	}
	if (this->_env_vars["SERVER_PROTOCOL"].empty())
		this->_flag = 505;
}

void	parsing::_parseserverPort(std::string &output, std::size_t &pos)
{
	std::size_t	i = 0, length_port = 0;

	if ((i = output.find(":", pos)) != std::string::npos)
	{
		i++;
		while (!std::isspace(output.at(i + length_port)))
			length_port++;
		this->_env_vars["SERVER_PORT"] = output.substr(i, length_port);
		pos += (i + 1 - pos) + length_port;
	}
}

void	parsing::_parseContentLength(std::string &output)
{
	std::size_t	i = 0, length_content_length = 0;

	if ((i = output.find("Content-Length: ", 0)) != std::string::npos)
	{
		i += 16;
		for (; std::isdigit(output[i + length_content_length]); length_content_length++);
		this->_content_length = output.substr(i, length_content_length);
		this->_length_body = atoi(_content_length.c_str());
	}
	else
	{
		this->_content_length = "-1";
		this->_length_body = 0;
		if (!this->_chunked)
			this->_flag = 411;
	}
	this->_env_vars["CONTENT_LENGTH"] = this->_content_length;
	if (this->_length_body > this->_block.getclientMaxBodySize())
		this->_flag = 413;
}

void	parsing::_parseContentType(std::string &output)
{
	std::size_t	i = 0, length_content_type = 0;

	if ((i = output.find("Content-Type: ", 0)) != std::string::npos)
	{
		i += 14;
		while (output.at(i + length_content_type) != '\r' && output.at(i + length_content_type) != '\n')
			length_content_type++;
		this->_content_type = output.substr(i, length_content_type);
	}
	else
		this->_content_type = "text/html";
	this->_env_vars["CONTENT_TYPE"] = this->_content_type;
}

void	parsing::_parseHttpAccept(std::string &output, std::string tofind)
{
	std::size_t	i = 0;
	std::size_t	length = 0;

	if ((i = output.find(tofind, 0)) != std::string::npos)
	{
		i += tofind.size() + 1;
		std::transform(tofind.begin(), tofind.end(), tofind.begin(), ::toupper);
		std::replace(tofind.begin(), tofind.end(), '-', '_');
		length = output.find("\r\n", i);
		tofind.erase(tofind.size()-1);
		this->_env_vars["HTTP_" + tofind] = output.substr(i, length - i);
	}
}

void	parsing::_parseTransferEncoding(std::string &output)
{
	if (output.find("Transfer-Encoding: chunked") != std::string::npos)
		this->_chunked = true;
	else
		this->_chunked = false;
}

void	parsing::_chooseconfig_handlerBeforeExecution(void)
{
	std::string	path;
	config_handler		tmpBlock = this->_block;

	if (this->_env_vars["SCRIPT_NAME"].empty())
		path = this->_env_vars["REQUEST_URI"];
	else
		path = this->_env_vars["REQUEST_URI"].substr(0, this->_env_vars["REQUEST_URI"].find_last_of("/") + 1);
	while (!path.empty())
	{
		config_handler	newconfig_handler;
		path = this->_getLocationBeforeExecution(path, tmpBlock, newconfig_handler);
	}
	if (this->_env_vars["SCRIPT_NAME"].empty() && !this->_block.getAutoIndex() && this->_env_vars["REQUEST_METHOD"].compare("DELETE"))
		this->_addIndex();
}

std::string	parsing::_getLocationBeforeExecution(std::string path, config_handler &tmpBlock, config_handler &newconfig_handler)
{
	std::map<std::string, config_handler>::iterator	iter;
	std::string	tmp = path;
	static bool	empty = false;

	while (!tmp.empty())
	{
		for (std::map<std::string, config_handler>::iterator it = tmpBlock.getLocation().begin(); it != tmpBlock.getLocation().end(); it++)
		{
			if (it->first == tmp)
			{
				newconfig_handler = it->second;
				tmpBlock = newconfig_handler;
				this->_changeBlockToNewconfig_handler(newconfig_handler);
				return path.substr(tmp.length(), path.length() - tmp.length());
			}
		}
		tmp = tmp.substr(0, tmp.find_last_of('/'));
	}
	if (empty == false)
	{
		empty = true;
		return "/";
	}
	return "";
}

void	parsing::_changeBlockToNewconfig_handler(config_handler &newconfig_handler)
{
	if (!newconfig_handler.getErrorPages().empty())
		this->_block.getErrorPages() = newconfig_handler.getErrorPages();
	if (this->_block.getclientMaxBodySize() != newconfig_handler.getclientMaxBodySize())
		this->_block.getclientMaxBodySize() = newconfig_handler.getclientMaxBodySize();
	if (this->_block.getcgiPass() != newconfig_handler.getcgiPass())
		this->_block.getcgiPass() = newconfig_handler.getcgiPass();
	if (!newconfig_handler.getAlowMethods().empty())
		this->_block.getAlowMethods() = newconfig_handler.getAlowMethods();
	if (this->_block.getRoot() != newconfig_handler.getRoot() && !newconfig_handler.getRoot().empty())
	{
		this->_block.getRoot() = newconfig_handler.getRoot();
		this->_env_vars["DOCUMENT_ROOT"] = this->_block.getRoot();
	}
	if (!newconfig_handler.getIndex().empty())
		this->_block.getIndex() = newconfig_handler.getIndex();
	if (newconfig_handler.getAutoIndex() == true)
		this->_block.getAutoIndex() = newconfig_handler.getAutoIndex();
	if (this->_block.getUploadFolder() != newconfig_handler.getUploadFolder())
		this->_block.getUploadFolder() = newconfig_handler.getUploadFolder();
	if (!newconfig_handler.getRedirection().first.empty())
		this->_block.getRedirection() = newconfig_handler.getRedirection();
}

void	parsing::_addIndex(void)
{
	for (size_t i = 0; i < this->_block.getIndex().size(); i++)
	{
		if (pathIsFile( this->_env_vars["DOCUMENT_ROOT"] + this->_env_vars["REQUEST_URI"] + this->_block.getIndex()[i]) == 1)
		{
			this->_env_vars["REQUEST_URI"].append(this->_block.getIndex()[i]);
			this->_parseScript(this->_env_vars["REQUEST_URI"]);
			if (this->_env_vars["DOCUMENT_ROOT"].compare("/") != 0)
				this->_env_vars["SCRIPT_FILENAME"] = this->_env_vars["DOCUMENT_ROOT"] + this->_env_vars["SCRIPT_NAME"];
			else
				this->_env_vars["SCRIPT_FILENAME"] = this->_env_vars["SCRIPT_NAME"];
			return ;
		}
	}
	if (pathIsFile(this->_env_vars["DOCUMENT_ROOT"] + this->_env_vars["REQUEST_URI"]) == 2)
	{
		this->_env_vars["DOCUMENT_ROOT"] = "./";
		this->_env_vars["SCRIPT_NAME"] = DEFAULT_INDEX;
		this->_env_vars["REQUEST_URI"] = this->_env_vars["SCRIPT_NAME"];
		this->_env_vars["SCRIPT_FILENAME"] = this->_env_vars["DOCUMENT_ROOT"] + this->_env_vars["SCRIPT_NAME"];
	}
}
