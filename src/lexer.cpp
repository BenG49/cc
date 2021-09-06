#include <lexer.hpp>

#include <cstring>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <string>
#include <limits>

const int STR_TOK_LEN = IDENTIFIER - 256;

const char *KEYWORDS[STR_TOK_LEN] = {
#define DEF(type, str) str "\0",
	TOKS
#undef DEF
};

const char *NAMES[TOK_COUNT - IDENTIFIER] = {
	"identifier",
	"integer constant",
	"floating constant",
	"character constant",
	"string constant",
};

const char CHAR_TOKENS[] = ";=-+*/,[](){}&|%!~<>^.?:";

char esc_code(char c)
{
	switch (c) {
		case 'a': return '\x07';
		case 'b': return '\x08';
		case 'e': return '\x1b';
		case 'f': return '\x0c';
		case 'n': return '\x0a';
		case 'r': return '\x0d';
		case 't': return '\x09';
		case 'v': return '\x0b';
		default: return c;
	}
}

Lexer::Lexer(const std::string &filename)
	: index(0), line(1), col(1)
{
	std::ifstream file(filename, std::ifstream::binary);

	if (!file)
		lex_err("Invalid file specified");

	// get len
	file.seekg(0, file.end);
	int len = file.tellg();
	file.seekg(0, file.beg);

	// alloc and set buf
	buf = new char[len + 1];
	file.read(buf, len);
	buf[len] = '\0';

	if (!file)
		lex_err("File could not be read!");

	file.close();

	// move past newlines
	count(0);
}

Lexer::~Lexer()
{
	delete[] buf;
}

Token Lexer::eat(TokType expected)
{
	Token next = pnxt();
	if (next.type != expected)
	{
		if (expected == ';')
			lex_err("Expected a \';\'");
		else
		{
			std::stringstream out;
		
			out << "Invalid token: expected "
				<< getname(expected)
				<< ", got "
				<< getname(next.type);

			lex_err(out.str());
		}
	}

	// remove cached value
	if (tok_buf.size() > 0)
		tok_buf.pop_front();
	
	return next;
}

Token Lexer::next()
{
	for (;;)
	{
		char cur = buf[index];

		// has to be first check, otherwise buffer overflow from buf[index + 1]
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
				while (buf[index + len] != '\n')
				{
					++len;
				}

				count(len);
				continue;
			}
			else if (next == '*')
			{
				blockcomment();
				continue;
			}
		}

		if (cur == '\'')
		{
			bool esc = false;
			char out = buf[index + 1];

			if (out == '\\')
			{
				esc = true;
				out = esc_code(buf[index + 2]);

				if (buf[index + 3] != '\'')
					lex_err("Unterminated character constant");
			}
			else if (out == '\'')
				lex_err("Character constant should contain at least one character");
			else if (buf[index + 2] != '\'')
				lex_err("Unterminated character constant");

			Token tok(CHAR_CONSTANT, line, col, esc ? 2 : 1);
			tok.val = static_cast<long long>(out);

			count(esc ? 4 : 3);
			return tok;
		}

		// check for numeric constant
		if (std::isdigit(cur) || cur == '.')
		{
			int end = index;
			int base = 10;
			bool fp = false;

			if (cur == '0')
			{
				cur = buf[++end];

				// hex
				if (cur == 'x' || cur == 'X')
				{
					do
						cur = buf[++end];
					while (std::isdigit(cur) || (cur >= 'a' && cur <= 'f') || (cur >= 'A' && cur <= 'F'));

					if (end - index < 3)
						lex_err("Invalid hex constant");
					else if (cur == '.')
						lex_err("Invalid floating constant");

					base = 16;
				}
				// binary
				else if (cur == 'b' || cur == 'B')
				{
					do
						cur = buf[++end];
					while (cur == '0' || cur == '1');

					if (end - index < 3)
						lex_err("Invalid binary constant");
					else if (cur == '.')
						lex_err("Invalid floating constant");

					base = 2;
				}
				// octal
				else
				{
					while (cur >= '0' && cur < '8')
						cur = buf[++end];

					if (cur == '8' || cur == '9')
						lex_err("Invalid octal constant");
					else if (cur == '.')
						lex_err("Invalid float constant");

					// not a single zero
					if (end - index > 1)
						base = 8;
				}
			}
			// dec
			else
			{
				do
				{
					if (cur == '.')
					{
						if (fp)
							lex_err("Invalid floating point constant: multiple \'.\'s");
						fp = true;
					}
					cur = buf[++end];
				} while (std::isdigit(cur) || cur == '.');

				// cannot have single '.'
				if (end - index == 1 && buf[index] == '.')
					goto NUMCHECK_END;
			}

			bool fsuffix = false;
			bool usuffix = false;
			int lcount = 0;

			// suffixes
			while (std::isalpha(cur))
			{
				if (cur == 'f')
				{
					// cant have two 'f's or unsigned float
					if (fsuffix || usuffix)
						lex_err("Extra text after expected end of number");
					fsuffix = true;
					fp = true;
				}
				else if (cur == 'u')
				{
					// can't have two 'u's or unsigned float
					if (usuffix || fp)
						lex_err("Extra text after expected end of number");
					usuffix = true;
				}
				else if (cur == 'l')
				{
					// can't have separated 'l's
					if (lcount)
						lex_err("Extra text after expected end of number");

					// long long
					if (buf[end + 1] == 'l')
					{
						lcount = 2;
						++end;
					}
					else
						lcount = 1;
				}
				
				cur = buf[++end];
			}

			int len = end - index;

			std::string numbuf;
			// for some reason stoll doesnt like 0b but works with 0x
			if (base == 2)
				numbuf = std::string(buf + index + 2, len - 2);
			else
				numbuf = std::string(buf + index, len);

			Token out(INT_CONSTANT, line, col, len);

			if (fp)
			{
				out.type = FP_CONSTANT;
				out.val = std::stod(numbuf);
			}
			else
				out.val = std::stoll(numbuf, 0, base);

			count(len);
			return out;
		} NUMCHECK_END:

		// check all keywords
		for (int i = 0; i < STR_TOK_LEN; ++i)
		{
			if (keyword(KEYWORDS[i]))
			{
				int len = std::strlen(KEYWORDS[i]);
				Token out((TokType)(i + 256), line, col, len);
				count(len);
				return out;
			}
		}

		// check single char constants
		const char *ptr = CHAR_TOKENS;
		while (*ptr)
			if (cur == *ptr++)
			{
				Token out((TokType)cur, line, col, 1);
				count(1);
				return out;
			}

		// check for string constant
		if (cur == '\"')
		{
			int esc_count = 0;
			int end = index;
			char prev = cur;

			do {
				prev = cur;
				cur = buf[++end];

				if (cur == '\\') ++esc_count;
			// \"([^\"\\\n]|\\.)*\"
			} while (cur && (prev == '\\' || cur != '\"') && cur != '\n');

			// reached EOF
			if (!cur)
				lex_err("Unterminated string");
			if (cur == '\n')
				lex_err("Missing closing quote");
			
			int buf_len = end - index - esc_count;
			int len = end - index + 1;
			std::string str_buf;
			str_buf.reserve(buf_len);

			int buf_idx = 0;
			// start at index + 1 to skip first quote
			for (int i = index + 1; i < end; ++i)
			{
				if (buf[i] == '\\')
					str_buf[buf_idx++] = esc_code(buf[++i]);
				else
					str_buf[buf_idx++] = buf[i];
			}

			Token out(STR_CONSTANT, line, col, len);
			out.val = str_buf;
			count(len);
			return out;
		}

		// try for a identifier
		if (std::isalpha(cur) || cur == '_')
		{
			int end = index;
			do
				cur = buf[++end];
			while (std::isalnum(buf[end]) || buf[end] == '_');

			int len = end - index;

			Token out(IDENTIFIER, line, col, len);
			out.val = std::string(buf + index, len);
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

// kinda scuffed method of finding if keywords arent bordering each other
bool Lexer::keyword(const char *keyword)
{
	bool is_alnum = std::isalnum(*keyword);
	// if prev char is num or alpha
	if (is_alnum && index > 0 && std::isalnum(buf[index - 1]))
		return false;

	const char *buf_ptr = buf + index;

	while (*keyword && *buf_ptr)
	{
		if (*keyword++ != *buf_ptr++)
			return false;
	}

	// if char after is num or alpha
	if (is_alnum && *buf_ptr && std::isalnum(*buf_ptr))
	{
		return false;
	}

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
	std::cerr << msg
			  << " at line " << line
			  << ", col " << col
			  << '\n';

	exit(1);
}

Token Lexer::pnxt() { return Lexer::peek(1); }
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

const char *Lexer::getname(TokType t)
{
	if (t < 256)
	{
		char *c = new char[2];
		c[0] = static_cast<char>(t);
		c[1] = '\0';
		return c;
	}
	else if (t < IDENTIFIER)
		return KEYWORDS[t - 256];
	else
		return NAMES[t - IDENTIFIER];
}

int Lexer::getsize(TokType t)
{
	switch (t) {
		case INT_CONSTANT: return 4;
		case CHAR_CONSTANT: return 1;
		default: return -1;
	}
}
