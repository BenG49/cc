#include <lexer.hpp>

#include <fstream>
#include <cctype>
#include <string>
#include <limits>
#include <regex>

#include <util/err.hpp>

const int STR_TOK_LEN = IDENTIFIER - 256;

const char *STR_TOKENS[STR_TOK_LEN] = {
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
	"return",
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
const std::regex STR_REGEX("(u8|[uUlL])?\"(\\.|[^\\\"\n])*\"");

Lexer::Lexer(const std::string &filename)
	: index(0)
	, line(1)
	, col(1)
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

		// has to be at the start, otherwise buffer overflow from buf[index + 1]
		if (cur == '\0')
			return Token(TOK_EOF, line, col, 0);

		// ignored characters
		if (cur == ' ' || cur == '\t' || cur == '\v' || cur == '\f')
		{
			count(1);
			continue;
		}

		// check for comments
		if (cur == '/')
		{
			char next = buf[index + 1];

			if (next == '/')
			{
				int len = 0;
				while (buf[index + len] != '\n') { ++len; }

				count(len);
				continue;
			}
			else if (next == '*')
			{
				blockcomment();
				continue;
			}
		}
		
		// check for numeric constant
		// c numeric constants are stupid complicated
		if (std::isdigit(cur) || cur == '.')
		{
			bool fp = false;
			int len = 0;
			int base = 10;

			// bin, hex, or octal
			if (cur == '0')
			{
				char next = buf[index + 1];

				// hex
				if (next == 'x' || next == 'X')
				{
					len += 2;
					// start past 0x
					cur = buf[index + len];
					char prev = cur;

					if (cur == '\'')
						lex_err("Digit separator cannot appear here");

					while (std::isdigit(cur) || (cur >= 'a' && cur <= 'f') || (cur >= 'A' && cur <= 'F') || cur == '\'')
					{
						if (cur == '\'' && prev == '\'')
							lex_err("Digit separator cannot appear here");

						prev = cur;
						cur = buf[index + (++len)];
					}
					
					if (len < 3)
						lex_err("Invalid hex constant");
					
					if (prev == '\'')
						lex_err("Digit separator cannot appear here");

					if (cur == '.')
						lex_err("Invalid floating constant");
					
					base = 16;
				}
				// bin
				else if (next == 'b' || next == 'B')
				{
					len += 2;
					// start past 0b
					cur = buf[index + len];
					char prev = cur;
					while (cur == '0' || cur == '1' || cur == '\'')
					{
						if (cur == '\'' && prev == '\'')
							lex_err("Digit separator cannot appear here");
						prev = cur;
						cur = buf[index + (++len)];
					}
					
					if (len < 3)
						lex_err("Invalid binary constant");

					if (prev == '\'')
						lex_err("Digit separator cannot appear here");

					if (cur == '.')
						lex_err("Invalid floating constant");

					base = 2;
				}
				// octal/0
				else
				{
					// start past 0
					char prev = cur;
					cur = buf[index + (++len)];
					while ((std::isdigit(cur) && cur < '8') || cur == '.' || cur == '\'')
					{
						if (cur == '\'' && (prev == '\'' || prev == '.'))
							lex_err("Digit separator cannot appear here");

						if (cur == '.')
						{
							if (prev == '\'')
								lex_err("Digit separator cannot appear here");

							// TODO: improve err msg
							if (fp)
								lex_err("Invalid floating point constant: multiple \'.\'s ");
							fp = true;
						}

						prev = cur;
						cur = buf[index + (++len)];
					}
					
					// if len == 1, then the number is 0, not octal
					if (len > 1)
					{
						if (prev == '\'')
							lex_err("Digit separator cannot appear here");

						// for some reason octal numbers are dec when floating
						if (!fp)
						{
							base = 8;

							if (cur == '8' || cur == '9')
								lex_err("Invalid octal constant");
						}
					}
				}
			}
			// base 10
			else
			{
				char prev = cur;
				while (std::isdigit(cur) || cur == '.')
				{
					if (cur == '\'' && (prev == '\'' || prev == '.'))
						lex_err("Digit separator cannot appear here");

					if (cur == '.')
					{
						if (prev == '\'')
							lex_err("Digit separator cannot appear here");

						// TODO: improve err msg
						if (fp)
							lex_err("Invalid floating point constant: multiple \'.\'s ");
						fp = true;
					}

					prev = cur;
					cur = buf[index + (++len)];
				}

				if (prev == '\'')
					lex_err("Digit separator cannot appear here");
				
				// if single '.'
				if (len == 1 && prev == '.')
					goto NUMCHECK_END;
			}

			bool fsuffix = false;
			bool sign = true;
			int longcount = 0;

			// numeric literal suffix
			while (std::isalpha(cur))
			{
				if (cur == 'f')
				{
					// cant have two 'f's or unsigned float or integer constant with f
					if (!fp || fsuffix || sign)
						lex_err("Extra text after expected end of number");

					fsuffix = true;
				}
				else if (cur == 'u')
				{
					// can't have two 'u's or unsigned float
					if (sign || fp)
						lex_err("Extra text after expected end of number");
					sign = true;
				}
				else if (cur == 'l')
				{
					// can't have separated 'l's
					if (longcount)
						lex_err("Extra text after expected end of number");

					// long long
					if (buf[index + len + 1] == 'l')
					{
						longcount = 2;
						++len;
					}
					else
						longcount = 1;
				}

				cur = buf[index + (++len)];
			}

			char *numbuf = new char[len + 1];
			memcpy(numbuf, buf + index, len);
			numbuf[len] = '\0';

			Token out(CONSTANT, line, col, len, {}, fp);

			// TODO: add integer type sizes
			// TODO: stoll doesn't work with digit separators :(
			if (!fp)
				out.val.i = std::stoll(numbuf, 0, base);
			else
			{
				out.val.f = std::stod(numbuf);
			}
			
			delete[] numbuf;
			count(len);
			return out;
		}
		NUMCHECK_END:

		// check single char constants
		const char *ptr = CHAR_TOKENS;
		while (*ptr)
			if (cur == *ptr++)
			{
				Token out(static_cast<TokType>(cur), line, col, 1);
				count(1);
				return out;
			}

		// check all keywords
		for (int i = 0; i < STR_TOK_LEN; ++i)
		{
			if (keyword(STR_TOKENS[i]))
			{
				int len = std::strlen(STR_TOKENS[i]);
				Token out(static_cast<TokType>(i + 256), line, col, len);
				count(len);
				return out;
			}
		}

		// try for a identifier
		if (std::isalpha(cur) || cur == '_')
		{
			int end = index;
			do
				cur = buf[++end];
			while (std::isalnum(buf[end]) || buf[end] == '_');

			int len = end - index;
			char *str = new char[len + 1];

			std::memcpy(str, buf + index, len);

			str[len] = '\0';

			Token out(IDENTIFIER, line, col, len, { .s =  str });
			count(len);
			return out;
		}
		
		std::string s("Invalid character \'");
		s += cur;
		s += '\'';
		lex_err(s);
	}
}

// assumes that there are no newlines between index and index + count
int Lexer::count(int count)
{
	index += count;
	col += count;

	char cur = buf[index];

	if (cur == '\n')
	{
		col = 1;

		while (cur == '\n')
		{
			cur = buf[++index];
			++line;
		}
	}

	if (cur == '\t')
		// 4 is the only respectable tab size
		col += 4 - (col & 0b11);

	return index;
}

bool Lexer::keyword(const char *keyword)
{
	const char *buf_ptr = buf + index;

	while (*keyword && *buf_ptr)
	{
		if (*keyword++ != *buf_ptr++)
			return false;
	}

	if (!*buf_ptr)
		return false;

	return true;
}

void Lexer::blockcomment()
{
	// 2 is for the /* to start the comment
	const char *ptr = buf + index + 3;
	char prev = *ptr;
	char cur = *ptr;

	int lines = 0;
	// number of columns after last newline
	int col_count = 1;

	while (cur)
	{
		++col_count;

		if (cur == '\n')
		{
			++lines;
			col_count = 1;
		}
		// found ending
		else if (prev == '*' && cur == '/')
		{
			//  +1 to move past '/'
			count(ptr - (buf + index) + 1);

			if (lines != 0)
			{
				col = col_count;
				line += lines;
			}

			return;
		}
		else if (prev == '/' && cur == '*')
			lex_err("Unterminated comment");

		// get char and go to next
		prev = *ptr++;
		cur = *ptr;
	}

	// no terminator, reaches end of file
	lex_err("Unterminated comment");
}

void Lexer::lex_err(const std::string &msg)
{
	std::cerr << "Error: " << msg
		<< " at line " << line
		<< ", col " << col
		<< '\n';

	exit(1);
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
