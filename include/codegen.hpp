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
extern const char *MOV[4];

void cg_err(const std::string &err);

Reg alloc_reg();
void free_reg(Reg reg);
void free_all();

// -------- codegen -------- //

Reg emit_mov(Reg a, Reg b, Size s);
Reg emit_int(int val);
Reg emit_unop(Reg val, TokType op);
Reg emit_binop(Reg src, Reg dst, TokType op);
Reg emit_div(Reg src, Reg dst, TokType op);
void emit_func_hdr(int sym, int scopeid, int offset);
void emit_epilogue();

// -------- gen -------- //
	
// reg = prev ast's output value
Reg gen_ast(AST *n, Reg reg, NodeType parent);

void init_cg(const std::string &filename);
