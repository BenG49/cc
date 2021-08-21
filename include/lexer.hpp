#pragma once

#include <cstdint>
#include <vector>

enum TokType {
	TOK_EOF,

	// first 255 are ascii symbols
	TOK_VAL_INT=256,
	TOK_VAL_FLOAT,
	TOK_ID_IF,
	TOK_ID_ELSE,
	TOK_ID_FOR,
	TOK_ID_WHILE,
	TOK_ID_CONST,
	TOK_TYPE_VOID,
	TOK_TYPE_INT,
	TOK_TYPE_FLOAT,
	TOK_TYPE_CHAR,
	TOK_TYPE_ENUM,
	TOK_TYPE_UNS,
	TOK_TYPE_LONG,
	TOK_ENUM_LEN
};

// starts at 256
const char *tok_names[TOK_ENUM_LEN - 256] = {
	"integer",
	"float",
	"\"if\"",
	"\"else\"",
	"\"for\"",
	"\"while\"",
	"\"const\"",
	"\"void\"",
	"\"int\"",
	"\"float\"",
	"\"char\"",
	"\"enum\"",
	"\"unsigned\"",
	"\"long\""
};

struct Token {
	union TokVal {
		std::uint64_t i;
		double f;
		const char *str;
	};

	TokType type;

	int line, col, char_count;

	union TokVal v;

	Token(TokType type, int line, int col, int char_count, union TokVal v = (TokVal){})
		: type(type), line(line), col(col), char_count(char_count), v(v) {}
};

class Lexer
{
	int index, line, col;
	char *buf;
	std::vector<Token> tok_buf;

	Token next();
	void inc_idx();
	void inc_idx_(int count);

public:
	Lexer(const std::string &filename);
	~Lexer();

	void eat(TokType expected);
	Token peek_next();
	// lookahead must be > 0
	Token peek(unsigned lookahead);
};
