#include <err.hpp>

const char *red = "\033[0;31m";
const char *nc = "\033[0m";

void err_tok(const std::string &msg, Token t)
{
	std::cerr << msg
			  << " at line " << t.line
			  << ", col " << t.col
			  << '\n';

	exit(1);
}

void err(const std::string &msg)
{
	std::cerr << msg << '\n';
	exit(1);
}

void warning(const std::string &msg)
{
	std::cerr << red << "warning" << nc << ": " << msg << '\n';
}
