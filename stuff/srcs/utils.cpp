# include "webserv.hpp"

std::vector<std::string> mySplit(std::string &str, std::string charset)
{
	std::vector<std::string>	ret;
	std::string::size_type		start;
	std::string::size_type		end = 0;
	std::string					tmp;

	str.push_back(charset[0]);
	start = str.find_first_not_of(charset, 0);
	while ((start = str.find_first_not_of(charset, end)) != std::string::npos)
	{
		end = str.find_first_of(charset, start);
		tmp = str.substr(start, end - start);
		ret.push_back(tmp);
	}
	return (ret);
}

int	pathIsFile(const std::string &path)
{
	struct stat	s;

	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return 2;
		else if (s.st_mode & S_IFREG)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

// DROITS FICHIERS

int	check_path(const std::string &path)
{
	struct stat	buf;

	if (stat(path.c_str(), &buf) == -1)
		return(-1);
	if (buf.st_mode & S_IFDIR)
		return (4);
	return (0);
}

int	check_read_rights(const std::string &path)
{
	struct stat	buf;

	if (stat(path.c_str(), &buf) == -1)
		return(-1);
	if (buf.st_mode & S_IRUSR)
		return (1);
	return (0);
}

int	check_wright_rights(const std::string &path)
{
	struct stat	buf;

	if (stat(path.c_str(), &buf) == -1)
		return(-1);
	if (buf.st_mode & S_IWUSR)
		return (1);
	return (0);
}

int	check_execute_rights(const std::string &path)
{
	struct stat	buf;

	if (stat(path.c_str(), &buf) == -1)
		return(-1);
	if (buf.st_mode & S_IXUSR)
		return (1);
	return (0);
}
