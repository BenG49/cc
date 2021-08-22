#include <iostream>
#include <string>

#include <util/err.hpp>
#include <lexer.hpp>

int main(int argc, const char *argv[])
{
	if (argc == 1)
		err("No input file specified");
	
	Lexer l(argv[1]);
	Token t = l.peek_next();
	while (t.type)
	{
		if (t.type && t.type < 256)
			std::cout << (char)t.type << '\n';
		else if (t.type == TOK_VAL_INT)
			std::cout << "int: " << t.val.i << '\n';
		else if (t.type == TOK_VAL_FLOAT)
			std::cout << "float: " << t.val.f << '\n';
		else
			std::cout << t.type << '\n';

		t = l.peek_next();
		l.eat(t.type);
	}
}

