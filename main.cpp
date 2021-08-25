#include <iostream>
#include <string>

#include <util/err.hpp>
#include <lexer.hpp>

int main(int argc, const char *argv[])
{
	if (argc == 1)
		err("No input file specified");
	
	Lexer l(argv[1]);

	/*Token t = l.peek_next();
	while(t.type)
	{
		t = l.peek_next();
		if (t.type > 255)
			std::cout << "TOK " << t.type;
		else
			std::cout << "TOK " << (char)t.type;

		if (t.type == CONSTANT)
		{
			if (t.fp)
				std::cout << " val " << t.val.f;
			else
				std::cout << " val " << t.val.i;
		}
		else if (t.type == IDENTIFIER || t.type == STRING_LITERAL)
			std::cout << " val \"" << t.val.s << '\"';

		std::cout << '\n';
		l.eat(t.type);
	}*/
}

