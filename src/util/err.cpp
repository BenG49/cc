#include <util/err.hpp>

void err(const std::string &msg)
{
	std::cerr << "Error: " << msg << '\n';

	exit(1);
}
