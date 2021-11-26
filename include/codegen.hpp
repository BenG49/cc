#pragma once

// based off of https://github.com/DoctorWkt/acwj/

#include <fstream>
#include <string>

#include <parser.hpp>
#include <scope.hpp>

enum Reg : int8_t;
// 1, 2, 4, 8 bytes
enum Size { Byte, Word, Long, Quad };
enum Idx { LE, GE, EQ, NE, LT, GT, UNCOND };

Size getsize(TokType t);

const Reg NOREG = static_cast<Reg>(-1);
// first function call register (ex. rdi on x86)
const Reg FIRST_ARG = static_cast<Reg>(6);

extern const int ARG_COUNT;

extern std::vector<std::pair<Sym, AST*>> globls;

// used to store information while doing gode generation
struct Ctx
{
	Reg reg;         // for binops, reg with lhs value
	NodeType parent; // parent node type
	int lbl;         // honestly i dont remember what this does
	int breaklbl;    // if in loop, label to break to
	int contlbl;     // if in loop, label to continue to

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

// marks reg as allocated
Reg alloc_reg();
void free_reg(Reg reg);
void free_all();
// get and inc global label number
int label();

// -------- codegen -------- //

// etc
void emit_jmp(int type, int lbl);
void emit_lbl(int lbl);
// move from one register to another, returns dst reg
Reg emit_mov(Reg src, Reg dst, Size s);
// move from reg onto stack given offset
void emit_mov(Reg src, int offset, Size s);
// move int value into register, return dst reg
Reg emit_int(int val, Size s);
void emit_push(Reg r);
void emit_pop(Reg r);
// perform widening operations (nothing on x86 since registers are long)
Reg emit_widen(Size oldtype, Size newtype, Reg r);

// computation
Reg emit_post(Reg val, NodeType op, const Sym &s);
Reg emit_unop(Reg val, NodeType op);
Reg emit_binop(Reg src, Reg dst, NodeType op);
Reg emit_div(Reg dst, Reg src, NodeType op);

// comparison and jumps
// compares a and b, sets a to 1 or 0 based on output, frees b
Reg cmp_set(Reg a, Reg b, NodeType op);
// compares a and b, jumps to lbl if satisfied, frees all regs
void cmp_jmp(Reg a, Reg b, NodeType op, int lbl);
// short circuit and/or
Reg logic_and_set(Reg a, AST *b, Ctx c);
Reg logic_or_set(Reg a, AST *b, Ctx c);
// eval node and jump to label in context if satisfied
void cond_jmp(AST *n, Ctx c);
void emit_call(const std::string &name);

// variables
// load variable into registre
Reg load_var(const Sym &s);
// set variable to r, return r
Reg set_var(Reg r, const Sym &s);
// called after codegen - emit all global directives
void gen_globls();

// stack and function management
void stack_alloc(int offset);
// size >= 0
void stack_dealloc(int size);
// global, func, prologue, stack alloc
void emit_func_hdr(const Sym &s, int offset);
void emit_epilogue();
// move r in to return register (if neccessary), return
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
