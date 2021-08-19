#include <lexer.hpp>
#include <iostream>

Lexer::Lexer(std::ifstream file)
{
	file.seekg(0, file.end);
	int len = file.tellg();
	file.seekg(0, file.beg);

	buf = new char[len];
	file.read(buf, len);

	if (!file)
	{
		std::cerr << "File could not be read!\n";
		exit(-1);
	}
}

Lexer::~Lexer()
{
	delete[] buf;
}

