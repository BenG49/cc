#include <codegen.hpp>
#include <parser.hpp>

const char *REG_64[REG_COUNT] = { "rax", "rbx", "rcx", "rdx", "rsp", "rbp", "rdi", "rsi", "rip" };
const char *REG_32[REG_COUNT] = { "eax", "ebx", "ecx", "edx", "esp", "ebp", "edi", "esi", "eip" };
const char *REG_16[REG_COUNT] = { "ax", "bx", "cx", "dx", "sp", "bp", "di", "si", "ip" };
const char *REG_H[GP_REGS_END] = { "ah", "bh", "ch", "dh" };
const char *REG_L[REG_COUNT - 1] = { "al", "bl", "cl", "dl", "spl", "bpl", "dil", "sil" };

static const char *get_cmp_set(TokType t)
{
	switch (t) {
		case OP_EQ: return "e";
		case OP_NE: return "ne";
		case OP_GE: return "ge";
		case OP_LE: return "le";
		case '<': return "l";
		case '>': return "g";
		default: return "";
	}
}

// -------- code generation -------- //

void Var::emit(Gen &g) const
{
	g.emit("movl -", false);
	g.emit_int(g.s.vec[entry].ebp_offset);
	g.emit_append("(%rbp), %eax", true);
}

void Block::emit(Gen &g) const
{
	for (Node *s : vec)
		s->emit(g);
}

void Func::emit(Gen &g) const
{
	g.emit_append(".globl ");
	g.emit_append(g.s.vec[name.entry].name.c_str(), true);

	g.emit_append(g.s.vec[name.entry].name.c_str());
	g.emit_append(":", true);

	// prologue
	g.emit("push %rbp");
	g.emit("mov %rsp, %rbp");

	if (offset)
	{
		// allocate space on the stack
		g.emit("sub $", false);
		g.emit_int(offset);
		g.emit_append(", %rsp", true);
	}

	g.emit("");

	blk->emit(g);

	// functions return 0 by default (standard in main, undefined for normal functions)
	g.emit("");
	g.emit("xor %eax, %eax");
	g.func_epilogue();
}

// all expressions MUST return in rax
void Ret::emit(Gen &g) const
{
	r->emit(g);
	g.func_epilogue();
}

void Decl::emit(Gen &g) const
{
	// if the declaration also initialized
	if (expr)
	{
		expr->emit(g);

		// move value into (ebp - offset)
		g.emit("mov %eax, -", false);
		g.emit_int(g.s.vec[v.entry].ebp_offset);
		g.emit_append("(%rbp)", true);
	}
}

// -------- expressions -------- //

void BinOp::emit(Gen &g) const
{
	// short circuit and, or
	if (op == OP_OR)
	{
		g.comment("short circuit or");
		const char *eval_rhs = g.get_label();
		const char *end = g.get_label();

		lhs->emit(g);

		g.emit("test %eax, %eax");
		g.emit("jz ", false); g.emit_append(eval_rhs, true);
		// g.emit("mov $1, %eax");
		g.emit("xor %eax, %eax");
		g.emit("inc %eax");
		g.emit("jmp ", false); g.emit_append(end, true);

		// eval rhs label
		g.emit_append(eval_rhs, false); g.emit_append(":", true);
		rhs->emit(g);
		g.emit("test %eax, %eax");
		g.emit("mov $0, %eax");
		g.emit("setnz %al");
		
		// end label
		g.emit_append(end, false); g.emit_append(":", true);

		delete[] eval_rhs;
		delete[] end;

		return;
	}
	else if (op == OP_AND)
	{
		g.comment("short circuit and");
		const char *eval_rhs = g.get_label();
		const char *end = g.get_label();

		lhs->emit(g);

		g.emit("test %eax, %eax");
		g.emit("jnz ", false); g.emit_append(eval_rhs, true);
		// set eax to zero: failed and
		g.emit("xor %eax, %eax");
		g.emit("jmp ", false); g.emit_append(end, true);

		// eval rhs label
		g.emit_append(eval_rhs, false); g.emit_append(":", true);
		rhs->emit(g);
		g.emit("test %eax, %eax");
		g.emit("mov $0, %eax");
		g.emit("setnz %al");
		
		// end label
		g.emit_append(end, false); g.emit_append(":", true);

		delete[] eval_rhs;
		delete[] end;

		return;
	}

	rhs->emit(g);

	// push rhs
	g.emit("push %rax");
	lhs->emit(g);
	// rbx=rhs
	g.emit("pop %rcx");

	switch (op) {
		case '-': g.emit("sub %ecx, %eax"); break;
		case '+': g.emit("add %ecx, %eax"); break;
		case '*': g.emit("imul %ecx, %eax"); break;

		case '/':
			// eax = (edx:eax)/ebx - quotient=edx
			g.emit("cdq");
			g.emit("idiv %ecx");
			break;

		case '%':
			// eax = (edx:eax)/ebx - quotient=edx
			g.emit("cdq");
			g.emit("idiv %ecx");
			// move remainder into result
			g.emit("mov %edx, %eax");
			break;
		
		case '|': g.emit("or %ecx, %eax"); break;
		case '^': g.emit("xor %ecx, %eax"); break;
		case '&': g.emit("and %ecx, %eax"); break;
		case OP_SHR: g.emit("sar %cl, %eax"); break;
		case OP_SHL: g.emit("shl %cl, %eax"); break;

		// == != >= > <= <
		default:
			if (op == OP_EQ || op == OP_NE || op == OP_GE || op == '>' || op == OP_LE || op == '<')
			{
				g.emit("cmp %eax, %ecx");
				g.emit("mov $0, %eax");

				g.emit("set", false);
				g.emit_append(get_cmp_set(op));
				g.emit_append(" %al", true);
			}
	};
}

void UnOp::emit(Gen &g) const
{
	switch (op) {
		case '-':
			operand->emit(g);
			g.emit("neg %eax");
			break;

		case '~':
			operand->emit(g);
			g.emit("not %eax");
			break;

		case '!':
			operand->emit(g);
			g.emit("test, %eax, %eax"); // test if operand is 0
			g.emit("mov $0, %eax"); // cannot use xor because it will reset flags
			g.emit("setz %al"); // if operand was zero, set to one
			break;
		default: break;
	}
}

void Assign::emit(Gen &g) const
{
	// eval rvalue
	rval->emit(g);

	if (op != '=')
	{

	g.emit("push %rax");
	lval->emit(g);
	g.emit("pop %rcx");

	switch (op) {
		case OP_ADD_SET: g.emit("add %ecx, %eax"); break;
		case OP_SUB_SET: g.emit("sub %ecx, %eax"); break;
		case OP_MUL_SET: g.emit("imul %ecx, %eax"); break;

		case OP_DIV_SET:
			// eax = (edx:eax)/ebx - quotient=edx
			g.emit("cdq");
			g.emit("idiv %ecx");
			break;

		case OP_MOD_SET:
			// eax = (edx:eax)/ebx - quotient=edx
			g.emit("cdq");
			g.emit("idiv %ecx");
			// move remainder into result
			g.emit("mov %edx, %eax");
			break;
		
		case OP_OR_SET: g.emit("or %ecx, %eax"); break;
		case OP_XOR_SET: g.emit("xor %ecx, %eax"); break;
		case OP_AND_SET: g.emit("and %ecx, %eax"); break;
		case OP_SHR_SET: g.emit("sar %cl, %eax"); break;
		case OP_SHL_SET: g.emit("shl %cl, %eax"); break;
		default: break;
	}

	}

	// move eax into (ebp - offset)
	g.emit("mov %eax, -", false);
	g.emit_int(g.s.vec[((Var*)lval)->entry].ebp_offset);
	g.emit_append("(%rbp)", true);
	return;
}

void Const::emit(Gen &g) const
{
	g.emit("mov $", false);
	g.emit_int(std::get<long long>(t.val));
	g.emit_append(", %eax", true);
}

// -------- gen -------- //

void Gen::x86_codegen(Block *ast) { ast->emit(*this); }

void Gen::emit(const char *str, bool nl)
{
	out << '\t' << str;
	if (nl) out << '\n';
}

void Gen::emit_append(const char *str, bool nl)
{
	out << str;
	if (nl) out << '\n';
}

void Gen::emit_int(long long i) { out << i; }
void Gen::emit_fp(double f) { out << f; }

void Gen::comment(const char *str)
{
	out << "# " << str << '\n';
};

void Gen::func_epilogue()
{
	out << "\n\tmov %rbp, %rsp\n"
		<< "\tpop %rbp\n"
		<< "\tret\n";
}

const char *Gen::get_label()
{
	char *buf = new char[10];
	// yes
	buf[0] = 'u';
	buf[1] = 'w';
	buf[2] = 'u';

	std::string tmp = std::to_string(label++);

	for (unsigned i = 0; i < tmp.size(); ++i)
		buf[i + 3] = tmp[i];

	buf[3 + tmp.size()] = '\0';

	return buf;
}
