#include <lexer.hpp>

#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <limits>
#include <regex>

#include <util/err.hpp>

const std::regex REGEX[TOKTYPE_LEN - 256] = {
	std::regex("^auto"),
	std::regex("^_Bool"),
	std::regex("^break"),
	std::regex("^case"),
	std::regex("^char"),
	std::regex("^_Complex"),
	std::regex("^const"),
	std::regex("^continue"),
	std::regex("^default"),
	std::regex("^do"),
	std::regex("^double"),
	std::regex("^else"),
	std::regex("^enum"),
	std::regex("^extern"),
	std::regex("^float"),
	std::regex("^for"),
	std::regex("^goto"),
	std::regex("^if"),
	std::regex("^_Imaginary"),
	std::regex("^inline"),
	std::regex("^int"),
	std::regex("^long"),
	std::regex("^register"),
	std::regex("^restrict"),
	std::regex("^short"),
	std::regex("^signed"),
	std::regex("^sizeof"),
	std::regex("^static"),
	std::regex("^struct"),
	std::regex("^switch"),
	std::regex("^typedef"),
	std::regex("^union"),
	std::regex("^unsigned"),
	std::regex("^void"),
	std::regex("^volatile"),
	std::regex("^while"),

	std::regex("^..."),
	std::regex("^>>="),
	std::regex("^<<="),
	std::regex("^+="),
	std::regex("^-="),
	std::regex("^*="),
	std::regex("^/="),
	std::regex("^%="),
	std::regex("^&="),
	std::regex("^^="),
	std::regex("^|="),
	std::regex("^>>"),
	std::regex("^<<"),
	std::regex("^++"),
	std::regex("^--"),
	std::regex("^->"),
	std::regex("^&&"),
	std::regex("^||"),
	std::regex("^<="),
	std::regex("^>="),
	std::regex("^=="),
	std::regex("^!="),
};

const char CHAR_TOKENS[] = ";=-+*/,[](){}&|%!~<>^.?:";

// FIXME: doesn't work completely, no f and potentially wrong
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


		std::cmatch m;
		int index = 0;
		for (int i = 256; i < TOKTYPE_LEN; ++i, ++index)
		{
			if (std::regex_search(buf + index, m, REGEX[index]))
			{
				
			}
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
