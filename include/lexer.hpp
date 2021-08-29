#pragma once

#include <iostream>
#include <string>
#include <deque>

#define TOKS                    \
	DEF(KEY_BOOL, "bool")         \
	DEF(KEY_CONST, "const")       \
	DEF(KEY_CHAR, "char")         \
	DEF(KEY_ELSE, "else")         \
	DEF(KEY_ENUM, "enum")         \
	DEF(KEY_FLOAT, "float")       \
	DEF(KEY_FOR, "for")           \
	DEF(KEY_IF, "if")             \
	DEF(KEY_INT, "int")           \
	DEF(KEY_RETURN, "return")     \
	DEF(KEY_SIZEOF, "sizeof")     \
	DEF(KEY_STRUCT, "struct")     \
	DEF(KEY_UNSIGNED, "unsigned") \
	DEF(KEY_VOID, "void")         \
	DEF(KEY_WHILE, "while")       \
	DEF(OP_SHR_SET, ">>=")        \
	DEF(OP_SHL_SET, "<<=")        \
	DEF(OP_ADD_SET, "+=")         \
	DEF(OP_SUB_SET, "-=")         \
	DEF(OP_MUL_SET, "*=")         \
	DEF(OP_DIV_SET, "/=")         \
	DEF(OP_MOD_SET, "%=")         \
	DEF(OP_AND_SET, "&=")         \
	DEF(OP_XOR_SET, "^=")         \
	DEF(OP_OR_SET, "|=")          \
	DEF(OP_SHR, ">>")             \
	DEF(OP_SHL, "<<")             \
	DEF(OP_INC, "++")             \
	DEF(OP_DEC, "--")             \
	DEF(OP_PTR, "->")             \
	DEF(OP_AND, "&&")             \
	DEF(OP_LE, "<=")              \
	DEF(OP_GE, ">=")              \
	DEF(OP_EQ, "==")              \
	DEF(OP_NE, "!=")

enum TokType
{
	TOK_EOF,
	LAST_ASCII = 255,
#define DEF(type, str) type,
	TOKS
#undef DEF

		IDENTIFIER,
	INT_CONSTANT,
	FP_CONSTANT,
	STR_CONSTANT,
	TOK_COUNT,
};

struct Token
{
	TokType type;

	int line, col, char_count;

	// TODO: make variant
	union {
		long long i;
		double f;
		std::string s;
	};

	bool fp;

	Token(TokType type, int line, int col, int char_count, bool fp = false)
		: type(type), line(line), col(col), char_count(char_count), fp(fp) {}
	
	Token(const Token &t)
		: type(t.type)
	{
		switch(type) {
			case INT_CONSTANT: i = t.i; break;
			case FP_CONSTANT:  f = t.f; break;
			case IDENTIFIER:
			case STR_CONSTANT: new(&s) std::string(t.s); break;
			default: break;
		}
	}
	
	~Token() {
		// for some reason this causes a double free/free of invalid memory, lol imagine worrying about mem leaks
		if (type == STR_CONSTANT || type == IDENTIFIER)
			s.~basic_string();
	};

	Token &operator=(const Token &t)
	{
		if (&t != this)
		{
			type = t.type;

			switch(type) {
				case INT_CONSTANT: i = t.i; break;
				case FP_CONSTANT:  f = t.f; break;
				case IDENTIFIER:
				case STR_CONSTANT: s = t.s; break;
				default: break;
			}
		}

		return *this;
	}
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

public:
	void lex_err(const std::string &msg);

	Lexer(const std::string &filename);
	~Lexer();

	void eat(TokType expected);
	Token peek_next();
	// lookahead must be > 0
	Token peek(unsigned lookahead);

	const char *getname(TokType t) const;
};
