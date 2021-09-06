#include <codegen.hpp>
#include <parser.hpp>

void Var::emit(Gen &g) const
{
	mov(true, g);
}

void Compound::emit(Gen &g) const
{
	if (scope->parent_scope)
		g.rbp(-(scope->vec.size() << 2));

	for (Node *s : vec)
		s->emit(g);
	
	// dont dealloc if function: just pop rbp
	if (!func && scope->parent_scope)
		g.rbp(scope->vec.size() << 2);
}

void If::emit(Gen &g) const
{
	const char *lbl_else = g.get_label();
	const char *else_end = nullptr;

	// cond
	cond->emit(g);

	// jz end/else
	g.emit("test %eax, %eax");
	g.jmp("jz", lbl_else);
	// if_blk
	if_blk->emit(g);

	// (if else_blk then jmp past else)
	if (else_blk)
	{
		else_end = g.get_label();
		g.jmp("jmp", else_end);
	}

	// else:
	g.label(lbl_else);
	
	// (if else_blk then emit else_blk, emit else_end:)
	if (else_blk)
	{
		else_blk->emit(g);
		g.label(else_end);
		delete[] else_end;
	}

	delete[] lbl_else;
}

void For::emit(Gen &g) const
{
	const char *start = g.get_label();
	const char *post_lbl = g.get_label();
	const char *end = g.get_label();

	g.break_lbl = end;
	g.cont_lbl = post_lbl;

	init->emit(g);

	g.label(start);

	if (cond->type != NONE)
	{
		cond->emit(g);
		g.emit("test %eax, %eax");
		g.jmp("jz", end);
	}

	blk->emit(g);

	g.label(post_lbl);

	post->emit(g);
	g.jmp("jmp", start);

	g.label(end);

	g.break_lbl = nullptr;
	g.cont_lbl = nullptr;
	
	delete[] start;
	delete[] end;
}

void ForDecl::emit(Gen &g) const
{
	const char *start = g.get_label();
	const char *post_lbl = g.get_label();
	const char *end = g.get_label();

	g.break_lbl = end;
	g.cont_lbl = post_lbl;

	// decl statement has its own scope, alloc var
	g.rbp(-(for_scope->vec.size() << 2));

	init->emit(g);

	g.label(start);

	if (cond->type != NONE)
	{
		cond->emit(g);
		g.emit("test %eax, %eax");
		g.jmp("jz", end);
	}

	blk->emit(g);
	g.label(post_lbl);
	post->emit(g);
	g.jmp("jmp", start);

	g.label(end);
	
	// decl statement has its own scope, dealloc var
	g.rbp(for_scope->vec.size() << 2);

	delete[] start;
	delete[] end;
}

void While::emit(Gen &g) const
{
	const char *start = g.get_label();
	const char *end = g.get_label();

	g.break_lbl = end;
	g.cont_lbl = start;

	g.label(start);

	// eval cond
	cond->emit(g);
	g.emit("test %eax, %eax");
	g.jmp("jz", end);

	// eval block
	blk->emit(g);
	g.jmp("jmp", start);

	g.label(end);

	delete[] start;
	delete[] end;
}

void Do::emit(Gen &g) const
{
	const char *start = g.get_label();
	const char *end = g.get_label();

	g.break_lbl = end;
	g.cont_lbl = start;

	g.label(start);

	// eval block
	blk->emit(g);

	// eval cond
	cond->emit(g);
	g.emit("test %eax, %eax");
	// if true, jump back to start otherwise continue
	g.jmp("jnz", start);

	g.label(end);

	delete[] start;
	delete[] end;
}

void Func::emit(Gen &g) const
{
	// forward declaration
	if (!blk)
		return;

	g.append("\n.globl ");
	g.append((*name.vars)[name.entry].name.c_str(), true);

	g.label((*name.vars)[name.entry].name.c_str());

	// prologue
	g.emit("push %rbp");
	g.emit("mov %rsp, %rbp");

	blk->emit(g);

	// functions return 0 by default (standard in main, undefined for normal functions)
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
	if (expr && !v.globl)
	{
		expr->emit(g);
		v.mov(false, g);
	}
}

// -------- expressions -------- //

void BinOp::emit(Gen &g) const
{
	// short circuit and, or
	if (op == OP_OR)
	{
		const char *eval_rhs = g.get_label();
		const char *end = g.get_label();

		lhs->emit(g);

		g.emit("test %eax, %eax");
		g.jmp("jz", eval_rhs);
		g.emit("mov $1, %eax");
		g.jmp("jmp", end);

		// eval rhs label
		g.label(eval_rhs);
		rhs->emit(g);
		g.emit("test %eax, %eax");
		g.emit("mov $0, %eax");
		g.emit("setnz %al");
		
		// end label
		g.label(end);

		delete[] eval_rhs;
		delete[] end;

		return;
	}
	else if (op == OP_AND)
	{
		const char *eval_rhs = g.get_label();
		const char *end = g.get_label();

		lhs->emit(g);

		g.emit("test %eax, %eax");
		g.jmp("jnz", eval_rhs);
		// set eax to zero: failed and
		g.emit("xor %eax, %eax");
		g.jmp("jmp", end);

		// eval rhs label
		g.label(eval_rhs);
		rhs->emit(g);
		g.emit("test %eax, %eax");
		g.emit("mov $0, %eax");
		g.emit("setnz %al");
		
		// end label
		g.label(end);

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
				g.emit("cmp %ecx, %eax");
				g.emit("mov $0, %eax");

				g.emit("set", false);

				switch (op) {
					case OP_EQ: g.append("e"); break;
					case OP_NE: g.append("ne"); break;
					case OP_GE: g.append("ge"); break;
					case OP_LE: g.append("le"); break;
					case '<': g.append("l"); break;
					case '>': g.append("g"); break;
					default: break;
				}

				g.append(" %al", true);
			}
	};
}

void UnOp::emit(Gen &g) const
{
	operand->emit(g);

	switch (op) {
		case '-': g.emit("neg %eax"); break;
		case '~': g.emit("not %eax"); break;
		case OP_INC: case OP_DEC:
			if (op == OP_DEC)
				g.emit("dec %eax");
			else
				g.emit("inc %eax");

			// update variable
			((Var*)operand)->mov(false, g);
			break;

		case '!':
			g.emit("test %eax, %eax"); // test if operand is 0
			g.emit("mov $0, %eax"); // cannot use xor because it will reset flags
			g.emit("setz %al"); // if operand was zero, set to one
			break;

		default: break;
	}
}

void Post::emit(Gen &g) const
{
	operand->emit(g);

	g.emit("push %rax");

	// apply op
	switch (op) {
		case OP_INC:
			g.emit("inc %eax");
			break;
		
		case OP_DEC:
			g.emit("dec %eax");
			break;

		default: break;
	}

	// update variable
	((Var*)operand)->mov(false, g);

	// reset register
	g.emit("pop %rax");
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
	((Var*)lval)->mov(false, g);
	return;
}

void Cond::emit(Gen &g) const
{
	const char *lbl_else = g.get_label();
	const char *else_end = g.get_label();

	// cond
	cond->emit(g);

	// jz false
	g.emit("test %eax, %eax");
	g.jmp("jz", lbl_else);
	// t
	t->emit(g);

	else_end = g.get_label();
	g.jmp("jmp", else_end);

	// false:
	g.label(lbl_else);
	
	f->emit(g);
	g.label(else_end);

	delete[] else_end;
	delete[] lbl_else;
}

// sysv calling convention
void Call::emit(Gen &g) const
{
	if (params.size())
	{
		// first 6 params go into registers
		for (unsigned i = 0; i < params.size() && i < 6; ++i)
		{
			params[i]->emit(g);

			// save prev reg - not very efficient, but works
			g.emit("push ", false);
			g.append(Gen::SYSV_REG_LIST[i], true);

			g.emit("mov %rax, ", false);
			g.append(Gen::SYSV_REG_LIST[i], true);
		}

		// rest are pushed from right to left
		for (int i = params.size() - 1; i >= 6; --i)
		{
			params[i]->emit(g);
			g.emit("push %rax");
		}
	}

	// call
	g.emit("call ", false);
	g.append((*func.vars)[func.entry].name.c_str(), true);

	for (int i = std::min(5, (int)params.size() - 1); i >= 0; --i)
	{
		// restore prev reg - not very efficient, but works
		g.emit("pop ", false);
		g.append(Gen::SYSV_REG_LIST[i], true);
	}

	// restore stack
	if (params.size() > 6)
	{
		g.emit("add $", false);
		g.emit_int((params.size() - 6) * 4);
		g.append(", %rsp", true);
	}
}

void Const::emit(Gen &g) const
{
	switch (t.type)
	{
		case INT_CONSTANT:
			g.emit("mov $", false);
			g.emit_int(std::get<long long>(t.val));
			g.append(", %eax", true);
			break;
		case KEY_BREAK:
			g.jmp("jmp", g.break_lbl);
			break;
		case KEY_CONT:
			g.jmp("jmp", g.cont_lbl);
			break;
		default: break;
	}
}

// -------- gen -------- //

void Gen::x86_codegen(Compound *ast)
{
	append(".data", true);

	// emit globals
	for (const Symbol &s : ast->scope->vec)
	{
		if (s.node->type == DECL)
		{
			Decl *d = (Decl*)s.node;
			const std::string &name = (*d->v.vars)[d->v.entry].name;

			// initialized block
			if (d->expr)
			{
				if (d->expr->type != CONST)
				{
					std::cerr << "Global variable must be initialized with constant\n";
					exit(1);
				}

				append(".globl ");
				append(name.c_str(), true);

				append(name.c_str());
				append(": ");
				append(".int ");
				emit_int(std::get<long long>(((Const*)d->expr)->t.val));
				nl();
			}
			// uninitialized block
			else
			{
				bool pass = false;

				// if forward declared, don't emit
				for (const Symbol &s : ast->scope->vec)
				{
					if (s.name == name && ((Decl*)s.node)->expr)
					{
						pass = true;
						break;
					}
				}

				if (pass)
					continue;

				append(".comm ");
				append(name.c_str());
				append(", ");
				emit_int(4);
				nl();
			}
		}
	}

	append("\n.text", true);

	ast->emit(*this);
}

void Gen::emit(const char *str, bool nl)
{
	out << '\t' << str;
	if (nl) out << '\n';
}

void Gen::append(const char *str, bool nl)
{
	out << str;
	if (nl) out << '\n';
}

void Gen::emit_int(long long i) { out << i; }
void Gen::label(const char *lbl) { out << lbl << ":\n"; }

void Gen::jmp(const char *jmp, const char *lbl)
{
	out << '\t' << jmp << ' ' << lbl << '\n';
}

void Gen::comment(const char *str)
{
	out << "# " << str << '\n';
};

void Gen::rbp(int offset)
{
	if (offset > 0)
		out << "\tsub $" << offset << ", %rsp\n";
	else if (offset < 0)
		out << "\tadd $" << offset << ", %rsp\n";
}

void Gen::nl() { out << '\n'; }

void Gen::func_epilogue()
{
	out << "\tmov %rbp, %rsp\n\tpop %rbp\n\tret\n";
}

const char *Gen::get_label()
{
	char *buf = new char[10];
	// yes
	buf[0] = 's';
	buf[1] = 'u';
	buf[2] = 's';

	std::string tmp = std::to_string(lbl_count++);

	for (unsigned i = 0; i < tmp.size(); ++i)
		buf[i + 3] = tmp[i];

	buf[3 + tmp.size()] = '\0';

	return buf;
}
const char *Gen::SYSV_REG_LIST[] = {
	"%rdi",
	"%rsi",
	"%rdx",
	"%rcx",
	"%r8",
	"%r9"
};
