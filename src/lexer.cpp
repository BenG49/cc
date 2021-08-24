#include <lexer.hpp>

#include <fstream>
#include <cctype>
#include <string>
// maybe later for stoi and stof
// #include <limits>
#include <regex>

#include <util/err.hpp>

const int STR_TOK_LEN = INT_LITERAL - 256;

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
const std::regex NUMERIC_LITERAL("^(0[xX][A-Fa-f0-9]+|0[bB][01]+|0[0-7]+|[0-9]*\\.?[0-9]+[fuLl]?)([uU]?[lL]{0,2})");
const std::regex STRING_LITERAL("(u8|[uUlL])?\"(\\.|[^\\\"\n])*\"");

// bad function, here for easier deletion later
void debug(std::string a)
{
	std::cout << a << '\n';
}

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
		debug("----------------");
		std::cout << " line " << line
		<< ", col " << col
		<< '\n';

		// has to be at the start, otherwise buffer overflow from buf[index + 1]
		if (cur == '\0')
			return Token(TOK_EOF, line, col, 0);

		// ignored characters
		if (cur == ' ' || cur == '\t' || cur == '\v' || cur == '\f')
		{
			debug("ignored");
			count(1);
			continue;
		}

		// check for comments
		if (cur == '/')
		{
			char next = buf[index + 1];

			if (next == '/')
			{
				debug("line comment");
				int len = 0;
				while (buf[index + len] != '\n') { ++len; }

				count(len);
				continue;
			}
			else if (next == '*')
			{
				debug("block comment");
				blockcomment();
				continue;
			}
		}

		// check single char constants
		const char *ptr = CHAR_TOKENS;
		while (*ptr)
			if (cur == *ptr++)
			{
				debug("single char literal");
				Token out(static_cast<TokType>(cur), line, col, 1);
				count(1);
				return out;
			}
		
		// check for numeric constant
		std::cmatch match;
		if ((std::isdigit(cur) || cur == '.') && std::regex_search(buf + index, match, NUMERIC_LITERAL))
		{
			debug("numeric constant");
			// TODO: add backreference checking or sth for check for float
			Token out(INT_LITERAL, line, col, match.str().size(), { .i = std::stoi(match.str()) });
			count(match.str().size());
			return out;
		}

		// check all keywords
		for (int i = 0; i < STR_TOK_LEN; ++i)
		{
			if (keyword(STR_TOKENS[i]))
			{
				debug("found token");
				int len = std::strlen(STR_TOKENS[i]);
				Token out(static_cast<TokType>(i + 256), line, col, len);
				count(len);
				return out;
			}
		}

		// try for a identifier
		if (std::isalpha(cur))
		{
			debug("trying identifier");
			int end = index;
			do
				cur = buf[++end];
			while (std::isalnum(buf[end]) || std::isdigit(buf[end]));

			int len = end - index;
			char *str = new char[len + 1];

			std::memcpy(str, buf + index, len);

			str[len] = '\0';

			std::cout << str << '\n';

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
			++index;
			++line;
		}
	}
	else if (cur == '\t')
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
	const char *ptr = buf + index + 2;
	char prev = *ptr;
	char cur = *ptr;

	int lines = 0;
	// number of columns after last newline
	int col_count = 0;

	while (cur)
	{
		++col_count;

		if (cur == '\n')
		{
			++lines;
			col_count = 0;
		}
		// found ending
		else if (prev == '*' && *ptr == '/')
		{
			count(buf + index - ptr);

			if (lines)
			{
				col = col_count;
				line += lines;
			}

			return;
		}
		else if (prev == '/' && *ptr == '*')
			lex_err("Unterminated comment");

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
