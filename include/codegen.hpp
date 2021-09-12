#pragma once

// based off of https://github.com/DoctorWkt/acwj/

#include <fstream>
#include <string>

#include <parser.hpp>

enum Reg {
	// scratch regs
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,

	// stack regs
	DI,
	SI,
	D,
	C,
	R8,
	R9,

	// etc
	A,
	B,

	// pointer regs
	BP,
	SP,
	IP, 

	COUNT,
	NOREG
};

enum Size { Byte, Word, Long, Quad };

Size getsize(TokType t);

const Reg ARG_START = DI;
const int SCRATCH_COUNT = 6;
extern const char *REGS[4][COUNT];

enum Idx { LE, GE, EQ, NE, LT, GT, UNCOND };

void cg_err(const std::string &err);

Reg alloc_reg();
void free_reg(Reg reg);
void free_all();
int label();

// -------- codegen -------- //

Reg emit_jmp(int type, int lbl);
void emit_lbl(int lbl);
Reg emit_mov(Reg a, Reg b, Size s);
Reg emit_int(int val);

Reg emit_unop(Reg val, TokType op);
Reg emit_binop(Reg src, Reg dst, TokType op);
Reg emit_div(Reg src, Reg dst, TokType op);

Reg cmp_set(Reg a, Reg b, TokType op);
void cmp_jmp(Reg a, Reg b, TokType op, int lbl);
Reg logic_and_set(Reg a, AST *b);
Reg logic_or_set(Reg a, AST *b);
void cond_jmp(AST *n, int lbl);

Reg load_var(const Sym &s);
Reg set_var(Reg r, const Sym &s);

void stack_alloc(int offset);
void stack_dealloc(int offset);
void emit_func_hdr(int sym, int scopeid, int offset);
void emit_epilogue();
void emit_ret();

// -------- gen -------- //
	
// reg = prev ast's output value
Reg gen_ast(AST *n, Reg reg, NodeType parent, int lbl=0);
void emit_if(AST *n);
Reg emit_cond(AST *n);

void init_cg(const std::string &filename);
