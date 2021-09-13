#pragma once

#include <iostream>
#include <variant>
#include <string>
#include <deque>

#define TOKS                      \
	DEF(KEY_BOOL, "bool")         \
	DEF(KEY_BREAK, "break")       \
	DEF(KEY_CONST, "const")       \
	DEF(KEY_CONT, "continue")     \
	DEF(KEY_CHAR, "char")         \
	DEF(KEY_DO, "do")			  \
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
	DEF(OP_LOGAND, "&&")          \
	DEF(OP_LOGOR, "||")           \
	DEF(OP_LE, "<=")              \
	DEF(OP_GE, ">=")              \
	DEF(OP_EQ, "==")              \
	DEF(OP_NE, "!=")

enum TokType
{
	TOK_EOF,
#define DEF(type, str) type,
	TOKS
#undef DEF
	// single char constants
	OP_LT,
	OP_GT,
	SEMI,
	OP_SET,
	OP_SUB,
	OP_ADD,
	OP_MUL,
	OP_DIV,
	COMMA,
	LBRAC_SQ,
	RBRAC_SQ,
	LPAREN,
	RPAREN,
	LBRAC,
	RBRAC,
	OP_AMPER,
	OP_OR,
	OP_MOD,
	OP_LOGNOT,
	OP_NOT,
	OP_XOR,
	DOT,
	OP_COND,
	OP_COLON,

	IDENTIFIER,
	INT_CONSTANT,
	FP_CONSTANT,
	CHAR_CONSTANT,
	STR_CONSTANT,
	TOK_COUNT,
};

struct Token
{
	TokType type;

	int line, col, char_count;

	std::variant<long long, double, std::string> val;

	Token(TokType type, int line, int col, int char_count)
		: type(type)
		, line(line)
		, col(col)
		, char_count(char_count) {}

	Token(const Token &t)
		: type(t.type)
		, line(t.line)
		, col(t.col)
		, char_count(t.char_count)
		, val(t.val) {}
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

	Token eat(TokType expected);
	// peek next
	Token pnxt();
	// lookahead must be > 0
	Token peek(unsigned lookahead);

	static const char *getname(TokType t);
};
