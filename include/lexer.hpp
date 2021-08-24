#pragma once

#include <iostream>
#include <cstdint>
#include <string>
#include <deque>

enum TokType {
	TOK_EOF,
	// ascii values

	KEY_AUTO = 256,
	KEY_BOOL,
	KEY_BREAK,
	KEY_CASE,
	KEY_CHAR,
	KEY_COMPLEX,
	KEY_CONST,
	KEY_CONTINUE,
	KEY_DEFAULT,
	KEY_DO,
	KEY_DOUBLE,
	KEY_ELSE,
	KEY_ENUM,
	KEY_EXTERN,
	KEY_FLOAT,
	KEY_FOR,
	KEY_GOTO,
	KEY_IF,
	KEY_IMAGINARY,
	KEY_INLINE,
	KEY_INT,
	KEY_LONG,
	KEY_REGISTER,
	KEY_RESTRICT,
	KEY_SHORT,
	KEY_SIGNED,
	KEY_SIZEOF,
	KEY_STATIC,
	KEY_STRUCT,
	KEY_SWITCH,
	KEY_TYPEDEF,
	KEY_UNION,
	KEY_UNSIGNED,
	KEY_VOID,
	KEY_VOLATILE,
	KEY_WHILE,

	OP_ELLIPSIS,
	OP_SHR_SET,
	OP_SHL_SET,
	OP_ADD_SET,
	OP_SUB_SET,
	OP_MUL_SET,
	OP_DIV_SET,
	OP_MOD_SET,
	OP_AND_SET,
	OP_XOR_SET,
	OP_OR_SET,
	OP_SHR,
	OP_SHL,
	OP_INC,
	OP_DEC,
	OP_PTR,
	OP_AND,
	OP_OR,
	OP_LE,
	OP_GE,
	OP_EQ,
	OP_NE,

	IDENTIFIER,
	INT_LITERAL,
	FLOAT_LITERAL,

	TOKTYPE_LEN
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

public:
	Token next();
	int count(int count);
	bool keyword(const char *keyword);
	void blockcomment();

	void lex_err(const std::string &msg);

// public:
	Lexer(const std::string &filename);
	~Lexer();

	void eat(TokType expected);
	Token peek_next();
	// lookahead must be > 0
	Token peek(unsigned lookahead);

	bool has_next() const;
};
