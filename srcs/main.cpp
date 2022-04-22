#include <iostream>
#include <cstdlib>
#include "server.hpp"
#include "color.hpp"

using namespace std;


int main(int ac, char const *av[])
{
	if (ac != 2)
		// exit(EXIT_FAILURE);
//	 av[1] = "./configuration_files/valid/default.conf"; // Pour ne pas avoir à ajouter de fichier config manuellement à chaque fois
	av[1] = "./configuration_files/valid/simple.conf"; // Pour ne pas avoir à ajouter de fichier config manuellement à chaque fois
	try
	{
		server webserv(av[1]);
		webserv.display_server(); // A utilité de check : peut être enlevé...
		webserv.run();
	}
	catch(const exception& e)
	{
		cerr << RED << e.what() << RESET << endl;
	}
	catch (...)
	{
		cout << "catch (...)\n" << endl;
	}
	
	return 0;
}
