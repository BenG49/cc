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

int lbl_n = 1;

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

Reg gen_ast(AST *n, Ctx c)
{
	switch (n->type) {
		case FUNC:
			if (n->rhs)
			{
				emit_func_hdr(n->lhs->get_sym(), n->val);
				gen_ast(n->rhs, Ctx(c, n->type));
				emit_epilogue();
			}
			return NOREG;
		case LIST:
			if (n->lhs) {
				gen_ast(n->lhs, Ctx(c, LIST));
				free_all();
				if (n->mid) {
					gen_ast(n->mid, Ctx(c, LIST));
					free_all();
					if (n->rhs)
					{
						gen_ast(n->rhs, Ctx(c, LIST));
						free_all();
					}
				}
			}

			// end of list
			if (n->val > 0)
				stack_dealloc(n->val);

			return NOREG;

		case DECL:
		case ASSIGN: {
			if (!n->rhs)
				return NOREG;
			
			Reg rval = gen_ast(n->rhs, Ctx(c, n->type));

			if (n->op != OP_SET)
			{
				Reg lval = gen_ast(n->lhs, Ctx(c, n->type));

				if (n->op == OP_SUB_SET || n->op == OP_SHR_SET || n->op == OP_SHL_SET)
					lval = emit_binop(rval, lval, n->op);
				else if (n->op == OP_DIV_SET || n->op == OP_MOD_SET)
					lval = emit_div(lval, rval, n->op);
				else
					lval = emit_binop(lval, rval, n->op);
				
				return set_var(lval, n->lhs->get_sym());
			}
			else
				// get rvalue, set lvalue
				return set_var(rval, n->lhs->get_sym());
		}

		case IF:
			gen_if(n, c);
			return NOREG;
		case COND:
			return gen_cond(n, c);
		case FOR:
		case FOR_DECL:
			gen_for(n, c);
			return NOREG;
		case WHILE:
			gen_while(n, c);
			return NOREG;
		case DO:
			gen_do(n, c);
			return NOREG;
		case BREAK:
			emit_jmp(UNCOND, c.breaklbl);
			return NOREG;
		case CONT:
			emit_jmp(UNCOND, c.contlbl);
			return NOREG;
		case CALL:
			return gen_call(n, c);

		default: break;
	}

	Reg l, r;

	if (n->lhs)
		l = gen_ast(n->lhs, Ctx(c, n->type));

	if (n->op == OP_AND)
		return logic_and_set(l, n->rhs, c);
	else if (n->op == OP_OR)
		return logic_or_set(l, n->rhs, c);

	if (n->rhs)
		r = gen_ast(n->rhs, Ctx(c, n->type, l));
	
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
				// if label is specified
				if (c.lbl)
				{
					cmp_jmp(l, r, n->op, c.lbl);
					return NOREG;
				}
				else
					return cmp_set(l, r, n->op);
			}
			else
				return emit_binop(l, r, n->op);

		case UNOP: {
			Reg r = emit_unop(l, n->op);

			if (n->op == OP_INC || n->op == OP_DEC)
				return set_var(r, n->lhs->get_sym());
			else
				return r;
		}

		case POSTFIX:
			return emit_post(l, n->op, n->lhs->get_sym());

		case RET:
			emit_ret(l, getsize(n->lhs->get_sym().type));
			free_all();
			return NOREG;

		case VAR: {
			Sym &s = n->get_sym();

			if (c.parent == DECL || (c.parent == ASSIGN && c.reg != NOREG))
				return set_var(c.reg, s);
			// not assigning
			else
				return load_var(s);
		}
	}

	return NOREG;
}

void gen_if(AST *n, Ctx c)
{
	int _false = label();
	int end;

	if (n->rhs)
		end = label();

	// if not cond, jump to false
	cond_jmp(n->lhs, Ctx(c, IF, _false));
	free_all();

	// generate true block
	gen_ast(n->mid, Ctx(c, IF));
	free_all();

	// jump to the end so that false block isn't executed
	if (n->rhs)
		emit_jmp(UNCOND, end);
	
	emit_lbl(_false);

	if (n->rhs)
	{
		// generate else block
		gen_ast(n->rhs, Ctx(c, IF));
		free_all();
		emit_lbl(end);
	}
}

Reg gen_cond(AST *n, Ctx c)
{
	int _false = label();
	int end = label();

	Reg out = alloc_reg();
	Reg r;

	// if not cond, jump to false
	cond_jmp(n->lhs, Ctx(c, COND, _false));
	free_all();

	// true block
	r = gen_ast(n->mid, Ctx(c, COND));
	emit_mov(r, out, Quad);
	// jump past false block
	emit_jmp(UNCOND, end);
	free_all();

	emit_lbl(_false);

	// false block
	r = gen_ast(n->rhs, Ctx(c, COND));
	emit_mov(r, out, Quad);

	emit_lbl(end);

	return out;
}

void gen_while(AST *n, Ctx c)
{
	int start = label();
	int end = label();

	emit_lbl(start);

	cond_jmp(n->lhs, Ctx(c, WHILE, end));

	// block
	gen_ast(n->rhs, Ctx(WHILE, end, start));
	// jump to start
	emit_jmp(UNCOND, start);

	emit_lbl(end);
}

void gen_for(AST *n, Ctx c)
{
	int start = label();
	int post = label();
	int end = label();

	// stack_alloc(-n->val);

	// init
	gen_ast(n->lhs, Ctx(c, FOR));
	emit_lbl(start);

	// gen conditional jump if the jump isn't none
	if (n->mid->type != NONE)
		gen_ast(n->mid, Ctx(c, FOR, end));

	// block
	gen_ast(n->rhs->lhs, Ctx(FOR, end, post));

	// post stmt
	emit_lbl(post);
	gen_ast(n->rhs->rhs, Ctx(c, FOR));

	// jump to start
	emit_jmp(UNCOND, start);

	emit_lbl(end);

	stack_dealloc(n->val);
}

void gen_do(AST *n, Ctx c)
{
	int start = label();
	int end = label();

	emit_lbl(start);

	// block
	gen_ast(n->rhs, Ctx(DO, end, start));
	// TODO: could be one jump if jnz to start
	cond_jmp(n->lhs, Ctx(c, DO, end));

	// jump to start
	emit_jmp(UNCOND, start);

	emit_lbl(end);
}

Reg gen_call(AST *n, Ctx c)
{
	bool pushed_regs[6] = {0};
	// push registers in use
	for (int i = 0; i < SCRATCH_COUNT; ++i)
		if (!free_regs[i])
		{
			emit_push(static_cast<Reg>(i));
			pushed_regs[i] = true;
		}

	ASTIter i(n->rhs);

	int offset = 0;
	int count = 0;
	while (i.has_next())
	{
		Reg r = gen_ast(i.next(), Ctx(c, CALL));

		if (count < 6)
		{
			Reg arg = static_cast<Reg>(SCRATCH_COUNT + count);
			emit_push(arg);
			emit_mov(r, arg, Quad);
		}
		else
			emit_mov(r, (offset -= 8), Quad);
		
		++count;
	}

	if (count > 6)
		stack_alloc(offset);
	
	emit_call(n->lhs->get_sym().name);

	if (count > 6)
		stack_dealloc(-offset);
	
	// pop saved regs
	for (int i = std::min(5, count - 1); i >= 0; --i)
		emit_pop(static_cast<Reg>(SCRATCH_COUNT + i));

	// pop prev saved regs
	for (int i = SCRATCH_COUNT - 1; i >= 0; --i)
		if (pushed_regs[i])
			emit_pop(static_cast<Reg>(i));
	
	Reg out = alloc_reg();

	return emit_mov(RR, out, Quad);
}
