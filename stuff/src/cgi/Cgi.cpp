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

// TODO erase this. is useless now
char	**Cgi::_create_env_tab(void)
{
	char		*tmp = NULL;
	char 		**env_tab = NULL;
	size_t		length = 0;
	size_t 		i = 0;
	//std::vector<std::string> tab;

	if (!(env_tab = (char **)malloc(sizeof(char *) * (this->_env_vars.size() + 1))))
		throw std::runtime_error("Error: Malloc\n");
	std::map<std::string, std::string>::iterator it = this->_env_vars.begin();
	for(;it != this->_env_vars.end(); it++) {
		//std::cout << it->first + " " + it->second << "\n";
		tmp = strdup(it->second.c_str());
		if (tmp == NULL)
			length = strlen(it->first.c_str()) + 1;
		else
			length = strlen(it->first.c_str()) + 2 + strlen(it->second.c_str());
		env_tab[i] = (char *)malloc(sizeof(char) * (length));
		env_tab[i] = strcpy(env_tab[i], it->first.c_str());
		if (tmp) {
			env_tab[i] = strcat(env_tab[i], "=\0");
			env_tab[i] = strcat(env_tab[i], tmp);
		}
		env_tab[i][length - 1] = '\0';
		//std::cout << i << " " << env_tab[i] << "\n";
		if (tmp)
			free(tmp);
		i++;
	}
	env_tab[this->_env_vars.size()] = 0;
	return env_tab;
}

void	Cgi::execute(void)
{
	//char		**tab = (char **)malloc(sizeof(char *) * 3);
	//char		**env_tab = NULL;
	pid_t		c_pid;
	FILE 		*fi, *fo;
	int			status = 0, fdi, fdo;
	char		*stab[3];
	char		*eenv_tab[this->_env_vars.size() + 1];
	std::string	s[this->_env_vars.size()];

	//if (!tab)
	//	throw std::runtime_error("Error: Malloc\n");
	std::map<std::string, std::string>::iterator it = this->_env_vars.begin();
	for(size_t i = 0;it != this->_env_vars.end(); ++it, ++i)
	{
		s[i] = (it->first + "=" + it->second);
		eenv_tab[i] = (char *)s[i].c_str();
		//std::cout << i << " " << s[i] << "\n";
	}
	eenv_tab[this->_env_vars.size()] = 0;
	//env_tab = this->_create_env_tab();
	// TODO erase _path_to_cgi DIS*
	_path_to_cgi = "/usr/bin/php-cgi";
	//std::cout << _path_to_cgi <<"\n";
	//tab[0] = strdup(this->_path_to_cgi.c_str());
	//tab[1] = strdup(this->_env_vars["SCRIPT_FILENAME"].c_str());
	//tab[2] = 0;
	stab[0] = (char *)(this->_path_to_cgi.c_str());
	stab[1] = (char *)(this->_env_vars["SCRIPT_FILENAME"].c_str());
	stab[2] = 0;
	//if (!tab[0] || !tab[1])
	//{
	//	if (!tab[1])
	//		free(tab[0]);
	//	free(tab);
	//	throw std::runtime_error("Error: Malloc\n");
	//}
	c_pid = fork();
	if (c_pid == 0)
	{
		fo = fopen(std::string("cgi_" + this->_infile).c_str(), "a");
		fdo = fileno(fo);
		//close(2);
		//std::cerr << "A\n";
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
	//for(size_t i = 0; tab[i]; i++)
	//	free(tab[i]);
	//free(tab);
	//for(size_t i = 0; env_tab[i]; i++)
	//	free(env_tab[i]);
	//free(env_tab);
}
