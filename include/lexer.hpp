#pragma once

#include <fstream>
#include <cstdint>
#include <string>

enum TokType {
	TOK_EOF,

	// first 255 are ascii symbols
	TOK_VAL_INTEGER=256,
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
};

struct Token {
	TokType type;

	int line, col;

	union val {
		std::uint64_t i;
		double f;
		const char *str;
	};
};

class Lexer
{
	char * buf;

public:
	Lexer(std::ifstream file);
	~Lexer();

	void eat(TokType expected);
	Token next();
	Token peek(unsigned lookahead);
};

