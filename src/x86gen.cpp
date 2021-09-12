#include <codegen.hpp>

// -------- codegen -------- //

const char *REGS[4][COUNT] = {
	{ "%r10b", "%r11b", "%r12b", "%r13b", "%r14b", "%r15b", "%dil", "%sil", "%dl",  "%cl",  "%r8l", "%r9l", "%al",  "%bl",  "%bpl", "%spl", "%ip" }, // 8
	{ "%r10w", "%r11w", "%r12w", "%r13w", "%r14w", "%r15w", "%di",  "%si",  "%dx",  "%cx",  "%r8w", "%r9w", "%ax",  "%bx",  "%bp",  "%sp",  "%ip" }, // 16
	{ "%r10d", "%r11d", "%r12d", "%r13d", "%r14d", "%r15d", "%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d", "%eax", "%ebx", "%ebp", "%esp", "%eip" }, // 32
	{ "%r10",  "%r11",  "%r12",  "%r13",  "%r14",  "%r15",  "%rdi", "%rsi", "%rdx", "%rcx", "%r8",  "%r9",  "%rax", "%rbx", "%rbp", "%rsp", "%rip" }, // 64
};

const char *MOV[4] = { "movb ", "movw ", "movl ", "movq " };
const char *CMP_SET[6] = { "setle ", "setge ", "sete ", "setne ", "setl ", "setg " };
const char *JMPS[7] = { "jg ", "jl ", "jne ", "je ", "jge ", "jle ", "jmp " };

std::ofstream out;

Reg emit_jmp(int type, int lbl)
{
	out << '\t' << JMPS[type] << 'L' << lbl << '\n';
}

void emit_lbl(int lbl)
{
	out << 'L' << lbl << ":\n";
}

Reg emit_mov(Reg src, Reg dst, Size s)
{
	if (src == dst)
		return src;

	out << '\t' << MOV[s] << REGS[s][src] << ", " << REGS[s][dst] << '\n';
	free_reg(src);
	return dst;
}

Reg emit_int(int val)
{
	Reg r = alloc_reg();
	out << "\tmovq $" << val << ", " << REGS[Quad][r] << '\n';
	return r;
}

// TODO: add inc and dec
Reg emit_unop(Reg val, TokType op)
{
	if (op == OP_NOT)
	{
		out << "\ttest " << REGS[Quad][val] << ", " << REGS[Quad][val] << '\n';
		out << "\tmovq $0, " << REGS[Quad][val] << '\n';
		out << "\tsetz " << REGS[Byte][val] << '\n';
		return val;
	}

	switch (op) {
		case OP_SUB: out << "\tneg "; break;
		case OP_BIT_NOT: out << "\tnot "; break;
	}

	out << REGS[Quad][val] << '\n';
	return val;
}

// note that sub, shr, and shl must be called with inverted args
Reg emit_binop(Reg src, Reg dst, TokType op)
{
	if (op == OP_SHR || op == OP_SHL)
		src = emit_mov(src, C, Quad);

	switch (op) {
		case OP_ADD: out << "\tadd "; break;
		case OP_SUB: out << "\tsub "; break;
		case OP_MUL: out << "\timul "; break;
		case OP_BIT_OR: out << "\tor "; break;
		case OP_BIT_AND: out << "\tand "; break;
		case OP_BIT_XOR: out << "\txor "; break;
		case OP_SHR: out << "\tsar "; break;
		case OP_SHL: out << "\tsal "; break;
	}

	out << REGS[Quad][src] << ", " << REGS[Quad][dst] << '\n';

	free_reg(src);

	return dst;
}

Reg emit_div(Reg src, Reg dst, TokType op)
{
	out << "\tmovq " << REGS[Quad][src] << ", %rax\n";

	out << "\tcqo\n\tidiv " << REGS[Quad][dst] << '\n';

	if (op == '/')
		out << "\tmovq %rax, " << REGS[Quad][dst] << '\n';
	else
		out << "\tmovq %rdx, " << REGS[Quad][dst] << '\n';

	free_reg(src);

	return dst;
}

Reg cmp_set(Reg a, Reg b, TokType op)
{
	out << "\tcmp " << REGS[Quad][b] << ", " << REGS[Quad][a] << '\n';
	out << "\tmov $0, " << REGS[Quad][a] << '\n';
	out << '\t' << CMP_SET[op - OP_LE] << REGS[Byte][a] << '\n';

	free_reg(b);

	return a;
}

void cmp_jmp(Reg a, Reg b, TokType op, int label)
{
	out << "\tcmp " << REGS[Quad][b] << ", " << REGS[Quad][a] << '\n';

	emit_jmp(op - OP_LE, label);

	free_all();
}

Reg logic_and_set(Reg a, AST *b)
{
	int second = label();
	int end = label();

	out << "\ttest " << REGS[Quad][a] << ", " << REGS[Quad][a] << '\n';
	emit_jmp(NE, second);
	out << "\txor " << REGS[Quad][a] << ", " << REGS[Quad][a] << '\n';
	emit_jmp(UNCOND, end);

	emit_lbl(second);
	Reg rhs = gen_ast(b, NOREG, BINOP);
	out << "\ttest " << REGS[Quad][rhs] << ", " << REGS[Quad][rhs] << '\n';
	out << "\tmov $0, " << REGS[Quad][a] << '\n';
	out << '\t' << CMP_SET[NE] << REGS[Byte][a] << '\n';

	emit_lbl(end);

	free_reg(rhs);
	return a;
}

Reg logic_or_set(Reg a, AST *b)
{
	int second = label();
	int end = label();

	out << "\ttest " << REGS[Quad][a] << ", " << REGS[Quad][a] << '\n';
	emit_jmp(EQ, second);
	out << "\tmovq $1, " << REGS[Quad][a] << '\n';
	emit_jmp(UNCOND, end);

	emit_lbl(second);
	Reg rhs = gen_ast(b, NOREG, BINOP);
	out << "\ttest " << REGS[Quad][rhs] << ", " << REGS[Quad][rhs] << '\n';
	out << "\tmov $0, " << REGS[Quad][a] << '\n';
	out << '\t' << CMP_SET[NE] << REGS[Byte][a] << '\n';

	emit_lbl(end);

	free_reg(rhs);
	return a;
}

void cond_jmp(AST *n, int lbl)
{
	Reg r = gen_ast(n, NOREG, IF, lbl);

	if (n->type != BINOP || n->op < OP_LE || n->op > OP_GT)
	{
		out << "\ttest " << REGS[Quad][r] << ", " << REGS[Quad][r] << '\n';
		out << '\t' << JMPS[NE] << 'L' << lbl << '\n';
	}
}

Reg load_var(const Sym &s)
{
	Reg r = alloc_reg();

	Size sz = getsize(s.type);

	out << '\t' << MOV[sz];

	switch (s.vtype) {
		case V_VAR:
			if (s.val) out << s.val;
			out << "(%rbp), ";
			break;

		case V_GLOBL:
			out << s.name << "(%rip), ";
			break;
	}

	out << REGS[sz][r] << '\n';
	return r;
}

Reg set_var(Reg r, const Sym &s)
{
	Size sz = getsize(s.type);

	out << '\t' << MOV[sz] << REGS[sz][r] << ", ";

	switch (s.vtype) {
		case V_VAR:
			if (s.val) out << s.val;
			out << "(%rbp)";
			break;

		case V_GLOBL:
			out << s.name << "(%rip)";
			break;
	}

	out << '\n';

	return r;
}

void stack_alloc(int offset)
{
	if (offset < 0)
		out << "\tsub $" << (-offset) << ", %rsp\n";
	else if (offset > 0)
		out << "\tadd $" << offset << ", %rsp\n";
}

void stack_dealloc(int offset)
{
	if (offset < 0)
		out << "\tadd $" << (-offset) << ", %rsp\n";
	else if (offset > 0)
		out << "\tsub $" << offset << ", %rsp\n";
}

void emit_func_hdr(int sym, int scopeid, int offset)
{
	Sym &s = Scope::s(scopeid)->syms[sym];

	out << ".globl " << s.name << '\n';
	out << s.name << ":\n";
	out << "\tpush %rbp\n\tmov %rsp, %rbp\n";

	stack_alloc(offset);
}

void emit_epilogue()
{
    out << "\txor %rax, %rax\n\tmov %rbp, %rsp\n\tpop %rbp\n\tret\n";
}

void emit_ret()
{
    out << "\tmov %rbp, %rsp\n\tpop %rbp\n\tret\n";
}


void init_cg(const std::string &filename)
{
	out.open(filename);

	if (!out)
		cg_err("Output file failed to open");
	
	free_all();
}
