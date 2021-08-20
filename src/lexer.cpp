#include <lexer.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <util/err.hpp>

Lexer::Lexer(const std::string &filename)
	: index(0)
	, line(0)
	, col(0)
{
	std::ifstream file(filename, std::ifstream::binary);

	if (!file)
		err("Invalid file specified");

	// get len
	file.seekg(0, file.end);
	int len = file.tellg();
	file.seekg(0, file.beg);

	// alloc and set buf
	buf = new char[len];
	file.read(buf, len);

	if (!file)
		err("File could not be read!");
	
	file.close();
}

Lexer::~Lexer()
{
	delete[] buf;
}


void Lexer::eat(TokType expected)
{
	Token next = Lexer::peek_next();
	if (next.type != expected)
	{
		std::stringstream out("Invalid token: expected ");
		out << tok_names[expected - 256];
		out << ", got ";
		out << tok_names[next.type - 256];

		err(out.str());
	}

	// move buffer index ahead
	index += next.char_count;

	// remove cached value
	// if (tok_buf.size() > 0)
	// 	tok_buf.();
}

Token Lexer::next()
{
	for (;;)
	{
		char cur = buf[index];

		// TOK_VAL_INT, TOK_VAL_FLOAT
		if (isdigit(cur) || cur == '.')
		{
			int start = index;
			bool f = false;

			while (isdigit(cur) || cur == '.')
			{
				if (cur == '.') f = true;

				cur = buf[index];
				inc_idx();
			}
			
			int len = index - start;
			char *num_buf = new char[len + 1];
			// TODO: check if this is right
			num_buf[len] = '\0';

			Token t = {
				.type = f ? TOK_VAL_FLOAT : TOK_VAL_INT,
				.line = line,
				.col = col,
				.char_count = len,
				.v = { .i = std::stoi(num_buf) }
			};

			delete[] num_buf;

			return t;
		}
	}
}

int Lexer::inc_idx()
{
	++index;
	++col;

	char cur = buf[index];
	
	if (cur == '\n')
	{
		++line;
		col = 0;
	}

	// if (cur == '\0')
	// {

	// }
}

Token Lexer::peek_next()
{
	return Lexer::peek(1);
}

Token Lexer::peek(unsigned lookahead)
{
	// token already exists in buffer
	if (lookahead < tok_buf.size())
		return tok_buf.at(lookahead);
	
	for (int i = tok_buf.size(); i < lookahead; ++i)
	{

	}
}
