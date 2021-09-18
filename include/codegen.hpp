#pragma once

// based off of https://github.com/DoctorWkt/acwj/

#include <fstream>
#include <string>

#include <parser.hpp>
#include <scope.hpp>

enum Reg {
	// scratch regs
	R0,
	R1,
	R2,
	R3,
	R4,
	R5,

	// stack regs
	A0,
	A1,
	A2,
	A3,
	A4,
	A5,
	// return reg
	RR,

	COUNT,
	NOREG
};

enum Size { Byte, Word, Long, Quad };
enum Idx { LE, GE, EQ, NE, LT, GT, UNCOND };

Size getsize(TokType t);

const int SCRATCH_COUNT = 6;
extern const char *REGS[4][COUNT];
extern std::vector<std::pair<Sym, AST*>> globls;

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

Reg alloc_reg();
void free_reg(Reg reg);
void free_all();
int label();

// -------- codegen -------- //

// etc
Reg emit_jmp(int type, int lbl);
void emit_lbl(int lbl);
Reg emit_mov(Reg src, Reg dst, Size s);
void emit_mov(Reg src, int offset, Size s);
Reg emit_int(int val, Size s);
void emit_push(Reg r);
void emit_pop(Reg r);
Reg emit_widen(Size oldtype, Size newtype, Reg r);

// computation
Reg emit_post(Reg val, NodeType op, const Sym &s);
Reg emit_unop(Reg val, NodeType op);
Reg emit_binop(Reg src, Reg dst, NodeType op);
Reg emit_div(Reg dst, Reg src, NodeType op);

// comparison and jumps
Reg cmp_set(Reg a, Reg b, NodeType op);
void cmp_jmp(Reg a, Reg b, NodeType op, int lbl);
Reg logic_and_set(Reg a, AST *b, Ctx c);
Reg logic_or_set(Reg a, AST *b, Ctx c);
void cond_jmp(AST *n, Ctx c);
void emit_call(const std::string &name);

// variables
Reg load_var(const Sym &s);
Reg set_var(Reg r, const Sym &s);
void gen_globls();

// stack and function management
void stack_alloc(int offset);
// size >= 0
void stack_dealloc(int size);
void emit_func_hdr(const Sym &s, int offset);
void emit_epilogue();
void emit_ret(Reg r, Size s);

// -------- gen -------- //
	
// reg = prev ast's output value
Reg gen_ast(AST *n, Ctx c);

void add_globl(const Sym &s, AST *val);

void gen_if(AST *n, Ctx c);
Reg gen_cond(AST *n, Ctx c);
void gen_while(AST *n, Ctx c);
void gen_for(AST *n, Ctx c);
void gen_do(AST *n, Ctx c);
Reg gen_call(AST *n, Ctx c);

void init_cg(const std::string &filename);
