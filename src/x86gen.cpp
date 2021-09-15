#include <codegen.hpp>

// -------- codegen -------- //

const char *REGS[4][COUNT] = {
	{ "%r10b", "%r11b", "%r12b", "%r13b", "%r14b", "%r15b", "%dil", "%sil", "%dl",  "%cl",  "%r8l", "%r9l", "%al"  }, // 8
	{ "%r10w", "%r11w", "%r12w", "%r13w", "%r14w", "%r15w", "%di",  "%si",  "%dx",  "%cx",  "%r8w", "%r9w", "%ax"  }, // 16
	{ "%r10d", "%r11d", "%r12d", "%r13d", "%r14d", "%r15d", "%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d", "%eax" }, // 32
	{ "%r10",  "%r11",  "%r12",  "%r13",  "%r14",  "%r15",  "%rdi", "%rsi", "%rdx", "%rcx", "%r8",  "%r9",  "%rax" }, // 64
};

const char *MOV[4] = { "movb ", "movw ", "movl ", "movq " };
const char *GLOBL_ALLOC[4] = { ".byte ", ".word ", ".long ", ".quad " };
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
	if (src < SCRATCH_COUNT) free_reg(src);
	return dst;
}

void emit_mov(Reg src, int offset, Size s)
{
	out << '\t' << MOV[s] << REGS[s][src] << ", ";

	if (offset) out << offset;
	out << "(%rbp)\n";
}

Reg emit_int(int val)
{
	Reg r = alloc_reg();
	out << "\tmovq $" << val << ", " << REGS[Quad][r] << '\n';
	return r;
}

void emit_push(Reg r)
{
	out << "\tpush " << REGS[Quad][r] << '\n';
}

void emit_pop(Reg r)
{
	out << "\tpop " << REGS[Quad][r] << '\n';
}

Reg emit_post(Reg val, NodeType op, const Sym &s)
{
	if (op == POST_INC)
		out << "\tinc " << REGS[Quad][val];
	else if (op == POST_DEC)
		out << "\tdec " << REGS[Quad][val];
	out << '\n';

	set_var(val, s);

	// undo val
	if (op == POST_INC)
		out << "\tdec " << REGS[Quad][val];
	else if (op == POST_DEC)
		out << "\tinc " << REGS[Quad][val];
	out << '\n';

	return val;
}

Reg emit_unop(Reg val, NodeType op)
{
	if (op == LOGNOT)
	{
		out << "\ttest " << REGS[Quad][val] << ", " << REGS[Quad][val] << '\n';
		out << "\tmovq $0, " << REGS[Quad][val] << '\n';
		out << "\tsetz " << REGS[Byte][val] << '\n';
		return val;
	}

	switch (op) {
		case NEG: out << "\tneg "; break;
		case NOT: out << "\tnot "; break;
		case UN_INC: out << "\tinc "; break;
		case UN_DEC: out << "\tdec "; break;
	}

	out << REGS[Quad][val] << '\n';

	return val;
}

// note that sub, shr, and shl must be called with inverted args
Reg emit_binop(Reg src, Reg dst, NodeType op)
{
	if (op == SHR || op == SHL)
		// arg 3 = rcx
		src = emit_mov(src, A3, Quad);

	switch (op) {
		case SET_ADD:
		case ADD: out << "\tadd "; break;
		case SET_SUB:
		case SUB: out << "\tsub "; break;
		case SET_MUL:
		case MUL: out << "\timul "; break;
		case SET_OR:
		case OR: out << "\tor "; break;
		case SET_AND:
		case AND: out << "\tand "; break;
		case SET_XOR:
		case XOR: out << "\txor "; break;
		case SET_SHR:
		case SHR: out << "\tsar "; break;
		case SET_SHL:
		case SHL: out << "\tsal "; break;
	}

	out << REGS[Quad][src] << ", " << REGS[Quad][dst] << '\n';

	free_reg(src);

	return dst;
}

Reg emit_div(Reg dst, Reg src, NodeType op)
{
	out << "\tmovq " << REGS[Quad][dst] << ", %rax\n";

	out << "\tcqo\n\tidiv " << REGS[Quad][src] << '\n';

	if (op == DIV || op == SET_DIV)
		out << "\tmovq %rax, " << REGS[Quad][dst] << '\n';
	else
		out << "\tmovq %rdx, " << REGS[Quad][dst] << '\n';

	free_reg(src);

	return dst;
}

Reg cmp_set(Reg a, Reg b, NodeType op)
{
	out << "\tcmp " << REGS[Quad][b] << ", " << REGS[Quad][a] << '\n';
	out << "\tmov $0, " << REGS[Quad][a] << '\n';
	out << '\t' << CMP_SET[op - N_LE] << REGS[Byte][a] << '\n';

	free_reg(b);

	return a;
}

void cmp_jmp(Reg a, Reg b, NodeType op, int label)
{
	out << "\tcmp " << REGS[Quad][b] << ", " << REGS[Quad][a] << '\n';

	emit_jmp(op - N_LE, label);

	free_all();
}

Reg logic_and_set(Reg a, AST *b, Ctx c)
{
	int second = label();
	int end = label();

	out << "\ttest " << REGS[Quad][a] << ", " << REGS[Quad][a] << '\n';
	emit_jmp(EQ, second);
	out << "\txor " << REGS[Quad][a] << ", " << REGS[Quad][a] << '\n';
	emit_jmp(UNCOND, end);

	emit_lbl(second);
	Reg rhs = gen_ast(b->rhs, Ctx(c, b->type));
	out << "\ttest " << REGS[Quad][rhs] << ", " << REGS[Quad][rhs] << '\n';
	out << "\tmov $0, " << REGS[Quad][a] << '\n';
	out << '\t' << CMP_SET[NE] << REGS[Byte][a] << '\n';

	emit_lbl(end);

	free_reg(rhs);
	return a;
}

Reg logic_or_set(Reg a, AST *b, Ctx c)
{
	int second = label();
	int end = label();

	out << "\ttest " << REGS[Quad][a] << ", " << REGS[Quad][a] << '\n';
	emit_jmp(NE, second);
	out << "\tmovq $1, " << REGS[Quad][a] << '\n';
	emit_jmp(UNCOND, end);

	emit_lbl(second);
	Reg rhs = gen_ast(b->rhs, Ctx(c, b->type));
	out << "\ttest " << REGS[Quad][rhs] << ", " << REGS[Quad][rhs] << '\n';
	out << "\tmov $0, " << REGS[Quad][a] << '\n';
	out << '\t' << CMP_SET[NE] << REGS[Byte][a] << '\n';

	emit_lbl(end);

	free_reg(rhs);
	return a;
}

void cond_jmp(AST *n, Ctx c)
{
	Reg r = gen_ast(n, c);

	if (n->type < SHR || n->type > XOR || n->type < N_LE || n->type > N_GT)
	{
		out << "\ttest " << REGS[Quad][r] << ", " << REGS[Quad][r] << '\n';
		out << '\t' << JMPS[NE] << 'L' << c.lbl << '\n';
	}
}

void emit_call(const std::string &name)
{
	out << "\tcall " << name << '\n';
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
		
		case V_REG:
			out << REGS[sz][s.val] << ", ";
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

		case V_REG:
			out << REGS[sz][s.val];
			break;
	}

	out << '\n';
	return r;
}

void gen_globls()
{
	if (globls.size())
		out << ".data\n";

	for (auto p : globls)
	{
		out << p.first.name << ": ";

		// initialized var
		if (!p.second)
			out << ".zero " << (1 << getsize(p.first.type)) << '\n';
		else
			out << GLOBL_ALLOC[getsize(p.first.type)] << ' ' << p.second->val << '\n';
	}
}

void stack_alloc(int offset)
{
	if (offset < 0)
		out << "\tsub $" << (-offset) << ", %rsp\n";
	else if (offset > 0)
		out << "\tadd $" << offset << ", %rsp\n";
}

void stack_dealloc(int size)
{
	if (size)
		out << "\tadd $" << size << ", %rsp\n";
}

void emit_func_hdr(const Sym &s, int offset)
{
	out << ".globl " << s.name << '\n';
	out << s.name << ":\n";
	out << "\tpush %rbp\n\tmov %rsp, %rbp\n";

	stack_alloc(offset);
}

void emit_epilogue()
{
    out << "\txor %rax, %rax\n\tmov %rbp, %rsp\n\tpop %rbp\n\tret\n";
}

void emit_ret(Reg r, Size s)
{
	if (r != RR)
		out << '\t' << MOV[s] << REGS[s][r] << ", " << REGS[s][RR] << '\n';
    out << "\tmov %rbp, %rsp\n\tpop %rbp\n\tret\n";
}

void init_cg(const std::string &filename)
{
	out.open(filename);

	if (!out)
		cg_err("Output file failed to open");
	
	free_all();
}
