#include <iostream>

int		main(int ac, char **av)
{
	std::cout << ac << std::endl;
	if (ac == 2)
		std::cout << av[1] << std::endl;
	return (0);
}
