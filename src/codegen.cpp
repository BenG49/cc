#include <codegen.hpp>
#include <parser.hpp>

#include <scope.hpp>

// true=free, false=allocated
bool free_regs[SCRATCH_COUNT] = { true };

Size getsize(TokType t)
{
	switch (t) {
		case KEY_INT: return Size::Long;
		case KEY_CHAR: return Size::Byte;
		default: return Size::Quad;
	}
}

const char *REGS[4][COUNT] = {
	{ "%r10b", "%r11b", "%r12b", "%r13b", "%r14b", "%r15b", "%dil", "%sil", "%dl",  "%cl",  "%r8l", "%r9l", "%al",  "%bl",  "%bpl", "%spl", "%ip" }, // 8
	{ "%r10w", "%r11w", "%r12w", "%r13w", "%r14w", "%r15w", "%di",  "%si",  "%dx",  "%cx",  "%r8w", "%r9w", "%ax",  "%bx",  "%bp",  "%sp",  "%ip" }, // 16
	{ "%r10d", "%r11d", "%r12d", "%r13d", "%r14d", "%r15d", "%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d", "%eax", "%ebx", "%ebp", "%esp", "%eip" }, // 32
	{ "%r10",  "%r11",  "%r12",  "%r13",  "%r14",  "%r15",  "%rdi", "%rsi", "%rdx", "%rcx", "%r8",  "%r9",  "%rax", "%rbx", "%rbp", "%rsp", "%rip" }, // 64
};

const char *MOV[4] = { "movb ", "movw ", "movl ", "movq " };
const char *CMP_SET[6] = { "setle ", "setge ", "sete ", "setne ", "setl ", "setg " };
const char *JMPS[7] = { "jg ", "jl ", "jne ", "je ", "jge ", "jle ", "jmp " };

enum Idx { LE, GE, EQ, NE, LT, GT, UNCOND };

// vars

std::ofstream out;
int lbl_n;

void cg_err(const std::string &err)
{
	std::cerr << err << '\n';
	exit(1);
}

// -------- register allocation -------- //

Reg alloc_reg()
{
	for (int i = 0; i < SCRATCH_COUNT; ++i)
	{
		if (free_regs[i])
		{
			free_regs[i] = false;
			return static_cast<Reg>(i);
		}
	}

	cg_err("Ran out of registers");
	// supress return type warning
	return NOREG;
}

void free_reg(Reg reg)
{
	if (free_regs[reg])
		cg_err("Attemted to free unallocated register");
	
	free_regs[reg] = true;
}

void free_all()
{
	for (int i = 0; i < SCRATCH_COUNT; ++i)
		free_regs[i] = true;
}

int label() { return lbl_n++; }

// -------- codegen -------- //

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

void emit_if(AST *n)
{
	int _false = label();
	int end;

	if (n->rhs)
		end = label();

	// if not cond, jump to false
	cond_jmp(n->lhs, _false);
	free_all();

	// generate true block
	gen_ast(n->mid, NOREG, IF);
	free_all();

	// jump to the end so that false block isn't executed
	if (n->rhs)
		emit_jmp(UNCOND, end);
	
	emit_lbl(_false);

	if (n->rhs)
	{
		// generate else block
		gen_ast(n->rhs, NOREG, IF);
		free_all();
		emit_lbl(end);
	}
}

Reg emit_cond(AST *n)
{
	int _false = label();
	int end = label();

	Reg out = alloc_reg();
	Reg r;

	// if not cond, jump to false
	cond_jmp(n->lhs, _false);
	free_all();

	// true block
	r = gen_ast(n->mid, NOREG, COND);
	emit_mov(r, out, Quad);
	free_all();

	emit_lbl(_false);

	// false block
	r = gen_ast(n->rhs, NOREG, COND);
	emit_mov(r, out, Quad);

	emit_lbl(end);

	return out;
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

Reg logic_and_jmp(Reg a, AST *b, int lbl)
{
}

Reg logic_or_jmp(Reg a, AST *b, int lbl)
{

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

void emit_func_hdr(int sym, int scopeid, int offset)
{
	Sym &s = Scope::s(scopeid)->syms[sym];

	out << ".globl " << s.name << '\n';
	out << s.name << ":\n";
	out << "\tpush %rbp\n\tmov %rsp, %rbp\n";

	if (offset < 0)
		out << "\tsub $" << (-offset) << ", %rsp\n";
	else if (offset > 0)
		out << "\tadd $" << offset << ", %rsp\n";
}

void emit_epilogue() { out << "\tmov %rbp, %rsp\n\tpop %rbp\n\tret\n"; }

// -------- gen -------- //

Reg gen_ast(AST *n, Reg reg, NodeType parent, int lbl)
{
	switch (n->type) {
		case FUNC:
			emit_func_hdr(n->lhs->val, n->lhs->scope_id, n->val);
			gen_ast(n->rhs, NOREG, n->type);
			out << "\txor %rax, %rax\n";
			emit_epilogue();
			return NOREG;
		case LIST:
			if (n->lhs) {
				gen_ast(n->lhs, NOREG, LIST);
				free_all();
				if (n->mid) {
					gen_ast(n->mid, NOREG, LIST);
					free_all();
					if (n->rhs)
					{
						gen_ast(n->rhs, NOREG, LIST);
						free_all();
					}
				}
			}
			return NOREG;

		case DECL:
		case ASSIGN:
			if (n->rhs)
			{
				Sym &s = Scope::s(n->lhs->scope_id)->syms[n->lhs->val];
				// get rvalue, set lvalue
				return set_var(gen_ast(n->rhs, NOREG, n->type), s);
			}
			return NOREG;

		case IF:
			emit_if(n);
			return NOREG;

		case COND:
			return emit_cond(n);

		default: break;
	}

	Reg l, r;

	if (n->lhs)
		l = gen_ast(n->lhs, NOREG, n->type);

	if (n->op == OP_AND)
		return logic_and_set(l, n->rhs);
	else if (n->op == OP_OR)
		return logic_or_set(l, n->rhs);

	if (n->rhs)
		r = gen_ast(n->rhs, l, n->type);
	
	switch (n->type) {
		case CONST:
			return emit_int(n->val);

		case BINOP:
			if (n->op == OP_SUB || n->op == OP_SHR || n->op == OP_SHL)
				return emit_binop(r, l, n->op);
			else if (n->op == OP_DIV || n->op == OP_MOD)
				return emit_div(l, r, n->op);
			else if (n->op >= OP_LE && n->op <= OP_GT)
			{
				if (parent == IF)
				{
					cmp_jmp(l, r, n->op, lbl);
					return NOREG;
				}
				else
					return cmp_set(l, r, n->op);
			}
			else
				return emit_binop(l, r, n->op);

		case UNOP:
			return emit_unop(l, n->op);

		case RET:
			emit_mov(l, A, getsize(Scope::s(n->lhs->scope_id)->syms[n->lhs->val].type));
			emit_epilogue();
			free_all();
			return NOREG;

		case VAR: {
			Sym &s = Scope::s(n->scope_id)->syms[n->val];

			if (parent == DECL || (parent == ASSIGN && reg != NOREG))
				return set_var(reg, s);
			// not assigning
			else
				return load_var(s);
		}
	}

	return NOREG;
}

void init_cg(const std::string &filename)
{
	out.open(filename);

	if (!out)
		cg_err("Output file failed to open");
	
	free_all();
}
