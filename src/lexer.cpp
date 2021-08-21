#include <lexer.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <util/err.hpp>

// starts at 256
const char *tok_names[TOK_ENUM_LEN - 256] = {
	"integer",
	"float",
	"if",
	"else",
	"for",
	"while",
	"const",
	"void",
	"int",
	"float",
	"char",
};

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

		if (cur == ' ' || cur == '\t' || cur == '\n')
			inc_idx();
		// TOK_VAL_INT, TOK_VAL_FLOAT
		else if (isdigit(cur) || cur == '.')
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

			Token t(
				f ? TOK_VAL_FLOAT : TOK_VAL_INT,
				line,
				col,
				len,
				(Token::TokVal){ .i = std::stoi(num_buf) }
			);

			delete[] num_buf;

			return t;
		}
		else
		// TOK_ID_IF, TOK_TYPE_INT
		else if (cur == 'i')
		{
			char next = buf[index + 1];
			if (next == 'f')
			{
				inc_seq(2);
				return Token(TOK_ID_IF, line, col, 2);
			}
			else if (seq_eq("int"))
			{
				inc_seq(3);
				return Token(TOK_TYPE_INT, line, col, 3);
			}
		}
		// TOK_ID_ELSE
		else if (seq_eq("else"))
		{
			inc_seq(4);
			return Token(TOK_ID_ELSE, line, col, 4);
		}
		// TOK_ID_FOR, TOK_TYPE_FLOAT
		else if (cur == 'f')
		{
			if (seq_eq("for"))
			{
				inc_seq(3);
				return Token(TOK_ID_FOR, line, col, 3);
			}
			else if (seq_eq("float"))
			{
				inc_seq(5);
				return Token(TOK_TYPE_FLOAT, line, col, 5);
			}
		}
		// TOK_ID_WHILE
		else if (seq_eq("while"))
		{
			inc_seq(5);
			return Token(TOK_ID_WHILE, line, col, 5);
		}
		// TOK_ID_CONST
		else if (seq_eq("const"))
		{
			inc_seq(5);
			return Token(TOK_ID_CONST, line, col, 5);
		}
		// TOK_ID_VOID
		else if (seq_eq("void"))
		{
			inc_seq(5);
			return Token(TOK_ID_CONST, line, col, 5);
		}
	}
}

void Lexer::inc_idx()
{
	inc_seq(1);
}

void Lexer::inc_seq(int count)
{
	index += count;
	col += count;
	char cur = buf[index];

	if (cur == '\n')
	{
		++line;
		col = 0;
	}
}

bool Lexer::seq_eq(const char *seq)
{
	int count = 0;

	while (*seq)
	{
		if (*seq++ != buf[index + count++])
			return false;
	}
	
	return true;
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
	
	for (unsigned i = tok_buf.size(); i < lookahead; ++i)
	{

	}
}
