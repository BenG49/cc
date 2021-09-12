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


// vars

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

// -------- gen -------- //

Reg gen_ast(AST *n, Reg reg, NodeType parent, int lbl)
{
	switch (n->type) {
		case FUNC:
			emit_func_hdr(n->lhs->val, n->lhs->scope_id, n->val);
			gen_ast(n->rhs, NOREG, n->type);
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

			// end of list
			if (n->val > 0)
				stack_dealloc(n->val);

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
			emit_ret();
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
	// jump past false block
	emit_jmp(UNCOND, end);
	free_all();

	emit_lbl(_false);

	// false block
	r = gen_ast(n->rhs, NOREG, COND);
	emit_mov(r, out, Quad);

	emit_lbl(end);

	return out;
}
