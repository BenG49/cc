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

struct Compound;

class Gen
{
	std::ofstream out;
	int lbl_count;

public:
	Gen(const std::string &outfile)
		: out(outfile)
		, lbl_count(0) {}

	void x86_codegen(Compound *ast);

	void emit(const char *str, bool nl=true);
	void emit_append(const char *str, bool nl=false);
	void emit_int(long long i);
	void emit_fp(double f);
	void label(const char *lbl);
	void jmp(const char *jmp, const char *lbl);
	void comment(const char *str);

	void func_epilogue();

	const char *get_label();

	// kinda bad but whatever
	const char *break_lbl;
	const char *cont_lbl;
};
