#pragma once

#include <fstream>
#include <string>

#include <symtab.hpp>

enum Reg {
	A, B, C, D, SP, BP, DI, SI, IP, REG_COUNT
};

const Reg GP_REGS_END = SP;

extern const char *REG_64[REG_COUNT];
extern const char *REG_32[REG_COUNT];
extern const char *REG_16[REG_COUNT];
extern const char *REG_H[GP_REGS_END];
// idk if eip has 8 bit counterpart
extern const char *REG_L[REG_COUNT - 1];

struct Block;

class Gen
{
	std::ofstream out;
	int label;

public:
	Gen(const std::string &outfile, SymTab &s)
		: out(outfile)
		, label(0)
		, s(s) {}

	void x86_codegen(Block *ast);

	void emit(const char *str, bool nl=true);
	void emit_append(const char *str, bool nl=false);
	void emit_int(long long i);
	void emit_fp(double f);
	void comment(const char *str);

	void func_epilogue();

	const char *get_label();
	
	const SymTab &s;
};
