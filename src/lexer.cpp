#include <lexer.hpp>

#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <limits>
#include <regex>

#include <util/err.hpp>

const TokType STR_TOK_LEN = TOKTYPE_LEN;

const char *STR_TOKENS[STR_TOK_LEN - 256] = {
	"auto",
	"_Bool",
	"break",
	"case",
	"char",
	"_Complex",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"enum",
	"extern",
	"float",
	"for",
	"goto",
	"if",
	"_Imaginary",
	"inline",
	"int",
	"long",
	"register",
	"restrict",
	"short",
	"signed",
	"sizeof",
	"static",
	"struct",
	"switch",
	"typedef",
	"union",
	"unsigned",
	"void",
	"volatile",
	"while",

	"...",
	">>=",
	"<<=",
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"&=",
	"^=",
	"|=",
	">>",
	"<<",
	"++",
	"--",
	"->",
	"&&",
	"||",
	"<=",
	">=",
	"==",
	"!=",
};

const char CHAR_TOKENS[] = ";=-+*/,[](){}&|%!~<>^.?:";

// FIXME: doesn't work completely, no f suffix and potentially wrong
const std::regex NUMERIC_LITERAL("(0[xX][A-Fa-f0-9]+|0[bB][01]+|0[0-7]+|[0-9]*\.?[0-9]+[fuLl]?)([uU]?[lL]{0,2})");

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
			/*<< REGEX[expected - 256]
			<< ", got "
			<< REGEX[next.type - 256]*/;

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

		// check single char constants
		const char *ptr = CHAR_TOKENS;
		while (*ptr)
			if (cur == *ptr++)
			{
				Token out(static_cast<TokType>(cur), line, col, 1);
				count();
				return out;
			}
		
		// check for numeric constant


		int index = 0;
		for (int i = 256; i < STR_TOK_LEN; ++i, ++index)
		{
		}
		
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

int Lexer::count() { return count_(1); }
// assumes that there are no newlines between index and index + count
int Lexer::count_(int count)
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
