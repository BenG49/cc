#pragma once

#include <iostream>
#include <cstdint>
#include <string>
#include <deque>

enum TokType {
	TOK_EOF,

	// first 255 are ascii symbols
	TOK_VAL_INT=256,
	TOK_VAL_FLOAT,
	TOK_IDENTIFIER,
	TOK_ID_IF,
	TOK_ID_ELSE,
	TOK_ID_FOR,
	TOK_ID_WHILE,
	TOK_ID_CONST,
	TOK_TYPE_VOID,
	TOK_TYPE_INT,
	TOK_TYPE_FLOAT,
	TOK_TYPE_CHAR,
	TOK_ENUM_LEN
};

struct Token {
	union TokVal {
		int i;
		std::uint64_t l;
		double f;
		const char *s;
	};

	TokType type;

	int line, col, char_count;

	union TokVal val;

	Token(TokType type, int line, int col, int char_count, union TokVal v = (TokVal){})
		: type(type), line(line), col(col), char_count(char_count), val(v) {}
};

class Lexer
{
	int index, line, col;
	char *buf;
	std::deque<Token> tok_buf;

	Token next();
	int inc_seq(int count);
	int inc_idx();
	bool seq_eq(const char *seq);

public:
	Lexer(const std::string &filename);
	~Lexer();

	void eat(TokType expected);
	Token peek_next();
	// lookahead must be > 0
	Token peek(unsigned lookahead);

	bool has_next() const;
};
