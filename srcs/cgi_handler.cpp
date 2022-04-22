#include "cgi_handler.hpp"
#include "color.hpp"

void debug_mp_out(std::map<std::string, std::vector<std::string> >& mp)
{
	for (std::map<std::string, std::vector<std::string> >::iterator it = mp.begin(); it != mp.end(); it++)
	{
		for (std::vector<std::string>::iterator it1 = it->second.begin(); it1 != it->second.end(); it1++)
			std::cout << it->first << "=" << *it1 << std::endl;
	}
}

bool	is_cgi(std::vector<std::string>& query)
{
	for (std::vector<std::string>::iterator it = query.begin(); it != query.end(); it++)
	{
		if ((it->find(".php")) != std::string::npos)
			return (true);
	}
	return (false);
}


static int		line_init(char **line, int offset, int ret)
{
	if (!(*line = (char*)malloc((offset + 1) * sizeof(char))))
		return (-1);
	((*line)[offset]) = '\0';
	return (ret);
}

static int		recursive_get_line(char **line, int fd, int offset)
{
	char	buffer;
	int		ret;

	ret = read(fd, &buffer, 1);
	if (ret == 1)
	{
		if (buffer == '\n' || buffer == '\0')
			return (line_init(line, offset, 1));
		else
		{
			ret = recursive_get_line(line, fd, offset + 1);
			if ((*line) != NULL)
				((*line)[offset]) = buffer;
			return (ret);
		}
	}
	else if (ret == 0)
		return (line_init(line, offset, 0));
	return (-1);
}

int				rec_gnl(int fd, char **line)
{
	char	buffer;

	if (fd < 0 || !line || read(fd, &buffer, 0) < 0)
		return (-1);
	*line = NULL;
	return (recursive_get_line(line, fd, 0));
}


void	go_cgi(std::map<std::string, std::vector<std::string> >& mp, const server_info& serv, int fd_in)
{
	cgi_handler	cgi(mp, serv);
//	cgi_handler	cgi(av);
	std::string	tmp;
	size_t			pos;
	int			fd[2];
	int			bfd[2];
	pid_t		pid;
	char		**plop = new char*[2];
	char		**lol = new char*[cgi.envSize() + 1];//{ (char*)"files/cgi/php-cgi", NULL };
	int	i;
//	std::string		out;

	plop[0] = (char*)CGI;
	plop[1] = NULL;
//	plop[1] = (char*)CGI_MODE;
//	plop[2] = NULL;
	i = 0;
	for (std::vector<std::string>::iterator it = cgi.envBegin(); it != cgi.envEnd(); it++, i++)
	{
		std::cout << GREEN << it->c_str() << RESET << std::endl;
		lol[i] = (char*)it->c_str();
	}
	lol[i] = NULL;
	if (pipe(fd) == -1)
		return ;
	if (pipe(bfd) == -1)
		return ;
//		return (CRASH_PIPE);
	pid = fork();
	if (pid == -1)
		return ;
//		return (CRASH_FORK);
	if (pid == 0)
	{
		close(fd[0]);
		if (!mp["BODY"].empty())
		{
			for (std::vector<std::string>::iterator it = mp["BODY"].begin(); it != mp["BODY"].end(); it++)
			{
				int post = it->size();
				std::cout << "post: " << post << std::endl;
				std::cout << "cLen: " << cgi._cLen << std::endl;
				if (post > cgi._cLen)
				{
					post = cgi._cLen;
					cgi._cLen = 0;
				}
				else
					cgi._cLen -= post;
				if (!it->empty())
				{
					write(bfd[1], it->c_str(), post);
//					write(2, it->c_str(), post);
				}
				if (cgi._cLen == 0)
					break ;
			}
		}
		close(bfd[1]);
		dup2(bfd[0], STDIN_FILENO);
		close(bfd[0]);
//		close(STDOUT_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		execve(plop[0],plop, lol);
		exit(1);
	}
	else
	{
		waitpid(pid, NULL, 0);
		close(bfd[0]);
		close(bfd[1]);
		if (!mp["BODY"].empty())
			mp["BODY"].clear();
		mp["BODY"] = std::vector<std::string>();
		char *sd;
		close(fd[1]);
		dup2(fd[0], fd_in);
		close(fd[0]);
		while (rec_gnl(fd_in, &sd) != 0)
		{
			tmp = (char*)sd;
			std::cout << tmp << std::endl;
			if ((pos = tmp.find("Content-type: ")) != std::string::npos)
			{
				tmp = tmp.replace(0, strlen("Content-type: "), "");
				mp["Content-Type:"].clear();
				mp["Content-Type:"].push_back(tmp);
			}
			else if (!((pos = tmp.find("X-Powered-By:")) != std::string::npos))
			{
				mp["BODY"].push_back(tmp);
			}
			free(sd);
		}
	}
	mp["A"].clear();
	/*
	mp["A"].erase(mp["A"].begin());
	mp["A"].erase(mp["A"].begin());
	mp["A"].push_back(" 200");
	mp["A"].push_back(" OK\r\n");
	*/
	delete [] lol;
}

cgi_handler::cgi_handler()	{
}

cgi_handler::cgi_handler(std::map<std::string, std::vector<std::string> >& mp, const server_info& serv)	: _cLen(0)	{
	extract_env(mp, serv);
}

cgi_handler::cgi_handler(const cgi_handler& other)	{
	handler = (char*)CGI;
	env = other.env;
}

cgi_handler::~cgi_handler()	{
	handler.clear();
	env.clear();
}

cgi_handler&	cgi_handler::operator=(const cgi_handler& other)	{
	(void)other;
	return (*this);
}

void	cgi_handler::extract_env(std::map<std::string, std::vector<std::string> >& mp,const server_info& _s)
{
	std::string			tmp;
	std::string			buf(mp["A"][1]);
	std::string			var;
	size_t				pos;

	tmp =  "REDIRECT_STATUS=CGI";
	env.push_back(tmp);
	tmp = "SERVER_SOFTWARE=";
	for (size_t j = 0; j < _s.server_name.size(); j++)
		tmp += _s.server_name[j];
	env.push_back(tmp);
	tmp = "SERVER_NAME=";
	tmp += _s.host;
	env.push_back(tmp);
	tmp = "GATEWAY_INTERFACE=CGI/1.1";
	env.push_back(tmp);
	tmp = "SERVER_PROTOCOL=HTTP/1.1";
	env.push_back(tmp);
	tmp = "SERVER_PORT=";
	tmp += _s.port;
	env.push_back(tmp);
	tmp = "REQUEST_METHOD=";
	tmp += mp["A"][0];
	env.push_back(tmp);
	tmp = "PATH_INFO=";
	pos = buf.find(".php");
	var = buf.substr(pos + 4);
	buf = buf.substr(0, pos + 4);
	if ((pos = var.find("/")) != std::string::npos)
		tmp += var.substr(pos + 1);
	env.push_back(tmp);
	tmp = "PATH_TRANSLATED=";
	tmp += mp["query"][0];
	env.push_back(tmp);
	tmp = "SCRIPT_NAME=";
//	tmp += buf;
	tmp += (buf[0] == '/' ? buf.substr(1) : buf);
	env.push_back(tmp);
	tmp = "QUERY_STRING=";
	if ((pos = var.find("?")) != std::string::npos)
		tmp += var.substr(pos + 1);
		/*
	if (!mp["BODY"].empty())
	{
		for (std::vector<std::string>::iterator it = mp["BODY"].begin(); it != mp["BODY"].end(); it++)
			tmp += *it;
		mp["BODY"].clear();
	}
	*/
	env.push_back(tmp);
	tmp = "REMOTE_HOST=";
	if (!mp["Host:"].empty())
	{
		for (size_t j = 0; j < mp["Host:"].size(); j++)
			tmp+= mp["Host:"][j];
	}
	env.push_back(tmp);
	tmp = "DOCUMENT_ROOT=";
	pos = buf.find_last_of("/");
	var = buf.substr(0, pos);
	if (var.substr(0, 2) == "./")
		var = var.substr(2);
	tmp += var;
	env.push_back(tmp);
	tmp = "AUTH_TYPE=";
	if (!mp["Authorization:"].empty())
	{
		tmp+= mp["Authorization:"][0];
	}
	env.push_back(tmp);
	tmp = "REMOTE_USER=";
	// GET USER NAME FROM SERVER
	env.push_back(tmp);
	tmp = "REMOTE_IDENT=";
	// GET USER ID FROM SERVER
	env.push_back(tmp);
	tmp = "CONTENT_TYPE=";
	if (!mp["Content-Type:"].empty())
	{
		for (size_t j = 0; j < mp["Content-Type:"].size(); j++)
			tmp+= mp["Content-Type:"][j];
	}
	env.push_back(tmp);
	tmp = "CONTENT_LENGTH=";
	if (!mp["Content-Length:"].empty())
	{
		for (size_t j = 0; j < mp["Content-Length:"].size(); j++)
			tmp+= mp["Content-Length:"][j];
		std::stringstream	ss(tmp.substr(strlen("CONTENT_LENGTH=")));
		ss >> _cLen;
	}
	else
		tmp+="0";
	env.push_back(tmp);
	tmp = "HTTP_ACCEPT=";
	if (!mp["Accept:"].empty())
	{
		for (size_t j = 0; j < mp["Accept:"].size(); j++)
			tmp+= mp["Accept:"][j];
	}
	env.push_back(tmp);
	tmp = "HTTP_ACCEPT_LANGUAGE=";
	if (!mp["Accept-Language:"].empty())
	{
		for (size_t j = 0; j < mp["Accept-Language:"].size(); j++)
			tmp+= mp["Accept-Language:"][j];
	}
	env.push_back(tmp);
	tmp = "HTTP_USER_AGENT=";
	if (!mp["User-Agent:"].empty())
	{
		for (size_t j = 0; j < mp["User-Agent:"].size(); j++)
			tmp+= mp["User-Agent:"][j];
	}
	env.push_back(tmp);
//	tmp = "HTTP_COOKIE=";
	//GET COOKIE FROM SERVER
	//C EST UN BONUS !!!
//	env.push_back(tmp);
	tmp = "HTTP_REFERER=";
	if (!mp["Referer"].empty())
	{
		for (size_t j = 0; j < mp["Referer"].size(); j++)
			tmp+= mp["Referer"][j];
	}
	env.push_back(tmp);
	/*
		cout << "auth_basic : " << _s.location[j].auth_basic << endl;
		cout << "auth_user_file : " << _s.location[j].auth_user_file << endl;
		cout << "autoindex : " << _s.location[j].autoindex << endl;
		cout << "return_directive : " << _s.location[j].return_directive << endl;
		cout << "allowed_method : ";
		for (size_t k = 0; k < _s.location[j].allowed_method.size(); k++)
			cout << _s.location[j].allowed_method[k] << (k < _s[i].location[j].allowed_method.size() - 1 ? ", " : "");
		cout << endl << "return : ";
		for (size_t k = 0; k < _s.location[j].retour.size(); k++)
			cout << _s.location[j].retour[k] << (k < _s.location[j].retour.size() - 1 ? ", " : "");
		cout << endl;
		*/
}
