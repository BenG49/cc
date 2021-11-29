#pragma once

enum NodeType {
	NONE,
	WIDEN,
	LIST,
	IF,
	FOR,
	FOR_DECL,
	WHILE,
	DO,
	DECL,
	DECL_SET,

	// assignment
	SET,
	SET_SHR,
	SET_SHL,
	SET_ADD,
	SET_SUB,
	SET_MUL,
	SET_DIV,
	SET_MOD,
	SET_AND,
	SET_XOR,
	SET_OR,

	// binary op
	SHR,
	SHL,
	LOGAND,
	LOGOR,
	N_LE,
	N_GE,
	N_EQ,
	N_NE,
	N_LT,
	N_GT,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	AND,
	OR,
	XOR,

	// unary op
	UN_INC,
	UN_DEC,
	LOGNOT,
	NOT,
	NEG,
	REF,
	PTR,

	// postfix exp
	POST_INC,
	POST_DEC,

	FUNC,
	CALL,
	RET,
	COND,
	VAR,
	BREAK,
	CONT,
	INT_CONST,

	NODE_COUNT
};

enum PrimType {
	NO_WIDEN,
	VOID, INT, CHAR, LONG,
	VOID_PTR, INT_PTR, CHAR_PTR, LONG_PTR,
	P_COUNT
};

extern const char *PRIM_NAMES[P_COUNT];
extern const char *NODE_NAMES[NODE_COUNT];

