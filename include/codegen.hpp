#pragma once

#include <fstream>
#include <string>

#include <lexer.hpp>

struct Compound;

class Gen
{
	std::ofstream out;
	int lbl_count;

public:
	Gen(const std::string &outfile)
		: out(outfile)
		, lbl_count(0) {}

	enum Reg { A, B, C, D, DI, SI, BP, SP, R8, R9, R10, R11, R12, R13, R14, R15, IP, COUNT };
	enum Size { Byte, Word, Long, Quad };

	void x86_codegen(Compound *ast);

	void emit(const char *str, bool nl=true);
	void emitc(char c);
	void emit_int(long long i);
	void append(const char *str, bool nl=false);

	void label(const char *lbl);
	void jmp(const char *jmp, const char *lbl);
	void emit_mov(Size size);
	void rbp(int offset);
	void func_epilogue();
	void emit_reg(Reg r, Size size);

	void comment(const char *str);
	void nl();

	const char *get_label();

	// kinda bad but whatever
	const char *break_lbl;
	const char *cont_lbl;


	static Size getsize(TokType t);

	static const Reg SYSV_REGS[];
	static const char *REGS[4][COUNT];
};
