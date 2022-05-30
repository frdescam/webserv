#include "webserv.hpp"
#include "server.hpp"

bool g_end = false;

void sig_handler(int signal_num)
{
	g_end = true;
	(void) signal_num;
}

int		main(int argc, char **argv)
{
	server	server;

	try
	{
		signal(SIGINT, sig_handler);
		if (argc == 2)
			server.config(argv[1]);
		else
			server.config(DEFAULT_CONFIG);
		if (!server.setup())
			server.run();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return (0);
}
