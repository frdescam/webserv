#include "../inc/webserv.hpp"
#include "./server/Server.hpp"

bool g_end = false;

void signal_handler(int signal_num)
{
	g_end = true;
	std::cout << std::endl;
	(void) signal_num;
	//static_cast<void>(signal_num);
}

int		main(int ac, char **av)
{
	std::cout << GREEN << "**use php-cgi instead of /usr/bin/php-cgi\n" << RESET;
	//std::string	t = "";
	//std::cout << t.compare("") << "\n";
	Server	server;

	try
	{
		signal(SIGINT, signal_handler);
		if (ac == 2)
			server.config(av[1]);
		else
			server.config(DEFAULT_CONFIG);
		if (!server.setup())
		{
			//while (!g_end)
				//g_end = server.run();
			server.run();
		}
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return (0);
}
