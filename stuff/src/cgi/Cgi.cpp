#include "Cgi.hpp"

Cgi::Cgi(void)
{}

Cgi::~Cgi(void)
{}

Cgi::Cgi(std::string path, bool post, std::string infile, std::map<std::string, std::string> env_vars):
	_post(post), _path_to_cgi(path), _infile(infile), _env_vars(env_vars)
{
}

Cgi::Cgi(Cgi const & other):
	_post(other._post), _path_to_cgi(other._path_to_cgi), _infile(other._infile),
	_env_vars(other._env_vars)
{}

Cgi & Cgi::operator=(Cgi const & other)
{
	if (this != &other)
	{
		_post = other._post;
		_path_to_cgi = other._path_to_cgi;
		_infile = other._infile;
		_env_vars = other._env_vars;
	}
	return *this;
}

// TODO erase _path_to_cgi DIS*
void	Cgi::execute(void)
{
	pid_t		c_pid;
	FILE 		*fi, *fo;
	int			status = 0, fdi, fdo;
	char		*stab[3];
	char		*eenv_tab[this->_env_vars.size() + 1];
	std::string	s[this->_env_vars.size()];

	std::map<std::string, std::string>::iterator it = this->_env_vars.begin();
	for(size_t i = 0;it != this->_env_vars.end(); ++it, ++i)
	{
		s[i] = (it->first + "=" + it->second);
		eenv_tab[i] = (char *)s[i].c_str();
	}
	eenv_tab[this->_env_vars.size()] = 0;
	_path_to_cgi = "/usr/bin/php-cgi";
	stab[0] = (char *)(this->_path_to_cgi.c_str());
	stab[1] = (char *)(this->_env_vars["SCRIPT_FILENAME"].c_str());
	stab[2] = 0;
	c_pid = fork();
	if (c_pid == 0)
	{
		fo = fopen(std::string("cgi_" + this->_infile).c_str(), "a");
		fdo = fileno(fo);
		//close(2);
		if (dup2(fdo, STDOUT_FILENO) == -1)
			perror("dup2");
		if (this->_post)
		{
			fi = fopen(this->_infile.c_str(), "r");
			fdi = fileno(fi);
			if (dup2(fdi, STDIN_FILENO) == -1)
				perror("dup2");
		}
		execve(this->_path_to_cgi.c_str(), stab, eenv_tab);
		exit(EXIT_SUCCESS);
	}
	else if (c_pid < 0)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else
		waitpid(c_pid, &status, 0);
}
