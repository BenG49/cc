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
	while(t.type)
	{
		t = l.peek_next();
		std::cout << "TOK: " << t.type;

		if (t.val.i)
			std::cout << " val " << t.val.i;
		else if (t.val.f)
			std::cout << " val " << t.val.f;
		else if (t.val.s)
			std::cout << " val " << t.val.s;

		std::cout << '\n';
		l.eat(t.type);
	}
}

