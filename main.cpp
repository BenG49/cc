#include <iostream>

#include <util/err.hpp>
#include <lexer.hpp>

int main(int argc, const char *argv[])
{
	if (argc == 1)
		err("No input file specified");
	
	Lexer l(argv[1]);
}

