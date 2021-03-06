#include <defs.hpp>

const char *PRIM_NAMES[P_COUNT] = {
	"",
	"void",
	"int",
	"char",
	"long",
	"void ptr",
	"int ptr",
	"char ptr",
	"long ptr",
};

// yes
const char *NODE_NAMES[NODE_COUNT] = {
	"none",
	"widen",
	"list",
	"if",
	"for",
	"fordecl",
	"while",
	"do",
	"decl",
	"decl_set",
	"=",
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
	"&&",
	"||",
	"<=",
	">=",
	"==",
	"!=",
	"<",
	">",
	"+",
	"-",
	"*",
	"/",
	"%",
	"&",
	"|",
	"^",
	"++",
	"--",
	"!",
	"~",
	"-",
	"&",
	"*",
	"++",
	"--",
	"func",
	"call",
	"ret",
	"cond",
	"var",
	"break",
	"cont",
	"int const"
};
