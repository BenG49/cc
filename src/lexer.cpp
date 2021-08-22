#include <lexer.hpp>

#include <iostream>

#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <limits>

#include <util/err.hpp>

// starts at 256
const char *TOK_NAMES[TOK_ENUM_LEN - 256] = {
	"integer",
	"float",
	"identifier",
	"if",
	"else",
	"for",	// 260
	"while",
	"const",
	"void",
	"int",
	"float",
	"char",
};

const TokType TOK_KEYWORDS_START = TOK_ID_IF;


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
		std::stringstream out;
		out << "Invalid token: expected "
			<< TOK_NAMES[expected - 256]
			<< ", got "
			<< TOK_NAMES[next.type - 256];

		err(out.str());
	}

	// remove cached value
	if (tok_buf.size() > 0)
		tok_buf.pop_front();
}

Token Lexer::next()
{
	for (;;)
	{
		char cur = buf[index];
		
		if (cur == ' ' || cur == '\t' || cur == '\n')
			inc_idx();
		// int, long, float, double
		else if (std::isdigit(cur) || cur == '.')
		{
			int start = index;
			bool f = false;

			do {
				if (cur == '.') f = true;
				cur = buf[inc_idx()];
			} while (std::isdigit(cur) || cur == '.');
			
			int len = index - start;
			char *num_buf = new char[len + 1];
			// copy the number into the buffer
			std::memcpy(num_buf, buf + start, len);

			num_buf[len] = '\0';

			Token t(TOK_VAL_INT, line, col, len);

			if (f)
			{
				t.type = TOK_VAL_FLOAT;
				t.val.f = std::stof(num_buf);
			}
			else
				t.val.i = std::stoi(num_buf);

			// delete temporary number buffer
			delete[] num_buf;

			return t;
		}
		else
		{
			int index = TOK_KEYWORDS_START - 256;
			// loop through all keywords
			for (int type = TOK_KEYWORDS_START; type < TOK_ENUM_LEN; ++type, ++index)
			{
				if (seq_eq(TOK_NAMES[index]))
				{
					int len = std::strlen(TOK_NAMES[index]);
					inc_seq(len);
					return Token(static_cast<TokType>(type), line, col, len);
				}
			}

			// get identifier
			if (std::islower(cur) || std::islower(cur) || cur == '_')
			{
				int start = index;
				
				// inc until end of identifier
				do cur = buf[inc_idx()];
				while (std::islower(cur) || std::islower(cur) || cur == '_' || std::isdigit(cur));

				int len = start - index;
				// TODO: figure out why deallocating in deconstructor of token throws double free
				char *str = new char[len + 1];

				// copy identifier
				std::memcpy(str, buf + start, len);

				str[len] = '\0';

				return Token(TOK_IDENTIFIER, line, col, len, (Token::TokVal){ .s = str });
			}

			if (cur == '\0')
				return Token(TOK_EOF, line, col, 0);
			
			// TODO: check if this is right
			Token out(static_cast<TokType>(cur), line, col, 1);
			inc_idx();
			return out;

			// none of keywords matched
			/*std::stringstream e;
			e << "Invalid character \'"
				<< cur
				<< "\' at line "
				<< line
				<< ", col "
				<< col;

			err(e.str());*/
		}
	}
}

int Lexer::inc_idx() { return inc_seq(1); }
// assumes that there are no newlines between index and index + count
int Lexer::inc_seq(int count)
{
	index += count;
	col += count;
	char cur = buf[index];

	if (cur == '\n')
	{
		++line;
		col = 0;
	}

	return index;
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

Token Lexer::peek_next() { return Lexer::peek(1); }
Token Lexer::peek(unsigned lookahead)
{
	// token doesn't exist in buffer
	if (lookahead >= tok_buf.size())
	{
		for (unsigned i = tok_buf.size(); i < lookahead; ++i)
			tok_buf.push_back(next());
	}

	return tok_buf.at(lookahead - 1);
}
