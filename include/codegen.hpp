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

struct Ctx
{
	Reg reg;
	NodeType parent;
	int lbl;
	int breaklbl;
	int contlbl;

	Ctx(Reg reg, NodeType parent, int lbl, int breaklbl, int contlbl)
		: reg(reg), parent(parent), lbl(lbl), breaklbl(breaklbl), contlbl(contlbl) {}
	Ctx(Ctx prev, NodeType parent)
		: Ctx(NOREG, parent, 0, prev.breaklbl, prev.contlbl) {}
	Ctx(Ctx prev, NodeType parent, Reg reg)
		: Ctx(reg, parent, 0, prev.breaklbl, prev.contlbl) {}
	Ctx(Ctx prev, NodeType parent, int lbl)
		: Ctx(NOREG, parent, lbl, prev.breaklbl, prev.contlbl) {}
	Ctx(NodeType parent, int breaklbl, int contlbl)
		: Ctx(NOREG, parent, 0, breaklbl, contlbl) {}
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

// etc
Reg emit_jmp(int type, int lbl);
void emit_lbl(int lbl);
Reg emit_mov(Reg a, Reg b, Size s);
Reg emit_int(int val);

// computation
Reg emit_post(Reg val, TokType op, const Sym &s);
Reg emit_unop(Reg val, TokType op);
Reg emit_binop(Reg src, Reg dst, TokType op);
Reg emit_div(Reg dst, Reg src, TokType op);

// comparison and jumps
Reg cmp_set(Reg a, Reg b, TokType op);
void cmp_jmp(Reg a, Reg b, TokType op, int lbl);
Reg logic_and_set(Reg a, AST *b, Ctx c);
Reg logic_or_set(Reg a, AST *b, Ctx c);
void cond_jmp(AST *n, Ctx c);

// variables
Reg load_var(const Sym &s);
Reg set_var(Reg r, const Sym &s);

// stack and function management
void stack_alloc(int offset);
// size >= 0
void stack_dealloc(int size);
void emit_func_hdr(const Sym &s, int offset);
void emit_epilogue();
void emit_ret();

// -------- gen -------- //
	
// reg = prev ast's output value
Reg gen_ast(AST *n, Ctx c);

void emit_if(AST *n, Ctx c);
Reg emit_cond(AST *n, Ctx c);
void emit_while(AST *n, Ctx c);
void emit_for(AST *n, Ctx c);
void emit_do(AST *n, Ctx c);

void init_cg(const std::string &filename);
