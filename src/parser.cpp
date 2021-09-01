#include <parser.hpp>

#include <symtab.hpp>

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

static bool is_type(TokType t)
{
	return t == KEY_BOOL || t == KEY_CHAR || t == KEY_INT || t == KEY_FLOAT || t == KEY_VOID;
}

static bool is_assign(TokType t)
{
	return t == '=' || t == OP_ADD_SET || t == OP_SUB_SET || t == OP_MUL_SET || t == OP_DIV_SET
		|| t == OP_MOD_SET || t == OP_AND_SET || t == OP_XOR_SET || t == OP_OR_SET
		|| t == OP_SHR_SET || t == OP_SHL_SET;
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
				g.emit("cmp %eax, %ebx");
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

// -------- grammars -------- //

// binop | assign
Expr *Parser::expr()
{
	if (is_assign(l.peek(2).type))
	 	return assign();
	else
		return binop();
}

// func | ret | expr
Node *Parser::statement()
{
	Node *out = nullptr;
	bool semi = true;

	switch (l.peek_next().type) {
		case KEY_RETURN: out = returnstatement(); break;
		case IDENTIFIER: case INT_CONSTANT: case FP_CONSTANT: case STR_CONSTANT: case '!': case '~': case '-': case '(':
			out = expr(); break;
		case KEY_VOID: case KEY_BOOL: case KEY_CHAR: case KEY_INT: case KEY_FLOAT:
		{
			TokType t = l.peek(3).type;
			if (t == '=' || t == ';')
				out = decl();
			else
			{
				out = function();
				semi = false;
			}
			break;
		}
		default:
			l.lex_err(std::string("Invalid token ") + l.getname(l.peek_next().type));
	}

	if (semi)
		l.eat(static_cast<TokType>(';'));

	return out;
}

// 'return' expr
Stmt *Parser::returnstatement()
{
	l.eat(KEY_RETURN);

	Ret *out = new Ret;

	out->r = expr();

	return out;
}

// type IDENTIFIER '(' (type IDENTIFIER,)* ')' '{' blk '}'
Stmt *Parser::function()
{
	Func *out = new Func;

	TokType t = l.peek_next().type;
	if (!is_type(t))
		l.lex_err("Expected function return type");

	// eat type
	l.eat(t);
		
	// get func name
	// s.vec.emplace_back(t, std::get<std::string>(l.eat(IDENTIFIER).val), -1);
	s.vec.push_back(Symbol(t, std::get<std::string>(l.eat(IDENTIFIER).val), -1));

	out->name = s.vec.size() - 1;

	l.eat(static_cast<TokType>('('));

	while (l.peek_next().type != ')')
	{
		t = l.peek_next().type;
		if (!is_type(t) && t != KEY_VOID)
			l.lex_err("Expected function param type");
		
		l.eat(t);

		// get param name
		std::string str = std::get<std::string>(l.eat(IDENTIFIER).val);

		// out->params.emplace_back(t, str, out->name);
		out->params.push_back(Symbol(t, str, out->name.entry));

		if (l.peek_next().type != ')')
			l.eat(static_cast<TokType>(','));
	}

	l.eat(static_cast<TokType>(')'));

	cur_func = out;

	out->blk = block();

	cur_func = nullptr;

	return out;
}

// stmt | '{' stmt* '}'
Stmt *Parser::block()
{
	Block *out = new Block;

	if (l.peek_next().type == '{')
	{
		l.eat(static_cast<TokType>('{'));
		while (l.peek_next().type && l.peek_next().type != '}')
				out->vec.push_back(statement());
		
		l.eat(static_cast<TokType>('}'));
	}
	else
		out->vec.push_back(statement());

	return out;
}

// int IDENTIFIER ('=' expr)?
Stmt *Parser::decl()
{
	TokType t = l.peek_next().type;
	l.eat(KEY_INT);

	std::string name = std::get<std::string>(l.eat(IDENTIFIER).val);

	for (Symbol &s : s.vec)
		if (s.name == name)
			l.lex_err("Redefinition of variable " + name);

	s.vec.emplace_back(t, name, cur_func->name.entry);

	// only int for now
	cur_func->offset += 4;

	s.vec.back().ebp_offset = cur_func->offset;

	Decl *out = new Decl(Var(s.lookup(s.vec.back().name)));

	if (l.peek_next().type == '=')
	{
		l.eat(static_cast<TokType>('='));
		out->expr = expr();
	}

	return out;
}

// -------- expressions -------- //

// IDENTIFIER assign expr
Expr *Parser::assign()
{
	Var *v = new Var(s.lookup(std::get<std::string>(l.eat(IDENTIFIER).val)));

	TokType t = l.peek_next().type;
	if (!is_assign(t))
		l.lex_err("Expected assignment operator");
	
	l.eat(t);

	return new Assign(t, v, expr());
}

/*

binary operations

*/

// op_or
Expr *Parser::binop()
{
	return op_or();
}

// op_and ('&&' op_and)*
Expr *Parser::op_or()
{
	Expr *out = op_and();

	TokType t = l.peek_next().type;

	while (t == OP_AND)
	{
		l.eat(OP_AND);

		BinOp *tmp = new BinOp(OP_AND, out, op_and());
		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// bitwise_or ('||' bitwise_or)*
Expr *Parser::op_and()
{
	Expr *out = bitwise_or();

	TokType t = l.peek_next().type;

	while (t == OP_OR)
	{
		l.eat(OP_OR);

		BinOp *tmp = new BinOp(OP_OR, out, bitwise_or());
		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// bitwise_xor ('|' bitwise_xor)*
Expr *Parser::bitwise_or()
{
	Expr *out = bitwise_xor();

	TokType t = l.peek_next().type;

	while (t == '|')
	{
		l.eat(static_cast<TokType>('|'));

		BinOp *tmp = new BinOp(static_cast<TokType>('|'), out, bitwise_xor());
		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// bitwise_and ('^' bitwise_and)*
Expr *Parser::bitwise_xor()
{
	Expr *out = bitwise_and();

	TokType t = l.peek_next().type;

	while (t == '^')
	{
		l.eat(static_cast<TokType>('^'));

		BinOp *tmp = new BinOp(static_cast<TokType>('^'), out, bitwise_and());
		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// equality ('&' equality)*
Expr *Parser::bitwise_and()
{
	Expr *out = equality();

	TokType t = l.peek_next().type;

	while (t == '&')
	{
		l.eat(static_cast<TokType>('&'));

		BinOp *tmp = new BinOp(static_cast<TokType>('&'), out, equality());
		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// comparison ((OP_EQ|OP_NE) comparison)*
Expr *Parser::equality()
{
	Expr *out = comparison();

	TokType t = l.peek_next().type;

	while (t == OP_EQ || t == OP_NE)
	{
		l.eat(t);

		BinOp *tmp = new BinOp(t, out, comparison());
		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// shift ((OP_LE|OP_GE|'<'|'>') shift)*
Expr *Parser::comparison()
{
	Expr *out = shift();

	TokType t = l.peek_next().type;

	while (t == OP_LE || t == OP_GE || t == '<' || t == '>')
	{
		l.eat(t);

		BinOp *tmp = new BinOp(t, out, shift());
		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// term ((OP_SHR|OP_SHL) term)*
Expr *Parser::shift()
{
	Expr *out = term();

	TokType t = l.peek_next().type;

	while (t == OP_SHL || t == OP_SHR)
	{
		l.eat(t);

		BinOp *tmp = new BinOp(t, out, term());
		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// factor (('+'|'-') factor)*
Expr *Parser::term()
{
	Expr *out = factor();

	TokType t = l.peek_next().type;

	while (t == '+' || t == '-')
	{
		l.eat(t);

		BinOp *tmp = new BinOp(t, out, factor());
		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// unop (('/'|'*'|'%') unop)*
Expr *Parser::factor()
{
	Expr *out = unop();

	TokType t = l.peek_next().type;

	while (t == '/' || t == '*' || t == '%')
	{
		l.eat(t);

		BinOp *tmp = new BinOp(t, out, unop());
		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// ('!' | '~' | '-') unop | primary
Expr *Parser::unop()
{
	TokType t = l.peek_next().type;
	if (t != '!' && t != '~' && t != '-')
			return primary();
	
	l.eat(t);

	UnOp *out = new UnOp(t, unop());

	return out;
}

// INT_CONSTANT | FP_CONSTANT | STR_CONSTANT | IDENTIFIER | '(' expr ')'
Expr *Parser::primary()
{
	Token tok = l.peek_next();
	TokType t = tok.type;

	if (t == INT_CONSTANT || t == FP_CONSTANT || t == STR_CONSTANT)
	{
		l.eat(t);
		return new Const(tok);
	}
	else if (t == IDENTIFIER)
		return new Var(s.lookup(std::get<std::string>(l.eat(IDENTIFIER).val)));
	else if (t == '(')
	{
		l.eat(static_cast<TokType>('('));
		Expr *out = expr();
		l.eat(static_cast<TokType>(')'));

		return out;
	}

	l.lex_err(std::string("Invalid expression ") + l.getname(t));
	return nullptr;
}

/*

main program block

*/

// statementlist = statement statementlist | statement EOF
Block *Parser::parse()
{
	Block *out = new Block;

	while (l.peek_next().type)
		out->vec.push_back(statement());
	
	return out;
}
