#pragma once

#include <fstream>
#include <string>

#include <symtab.hpp>

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
	void append(const char *str, bool nl=false);
	void emit_int(long long i);
	void label(const char *lbl);
	void jmp(const char *jmp, const char *lbl);
	void comment(const char *str);
	void emitc(char c);
	void rbp(int offset);
	void nl();

	void func_epilogue();

	const char *get_label();

	// kinda bad but whatever
	const char *break_lbl;
	const char *cont_lbl;

	static const char *SYSV_REG_LIST[];
	static const char *AX_SIZE[];
	static const char MOV_SIZE[];
};
