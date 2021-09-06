#include <parser.hpp>

#include <symtab.hpp>

static bool is_type(TokType t)
{
	return t == KEY_BOOL || t == KEY_CHAR || t == KEY_INT || t == KEY_FLOAT || t == KEY_VOID;
}

static bool is_assign(TokType t)
{
	return t == '=' || t == OP_ADD_SET || t == OP_SUB_SET || t == OP_MUL_SET
		|| t == OP_DIV_SET || t == OP_MOD_SET || t == OP_AND_SET || t == OP_XOR_SET
		|| t == OP_OR_SET || t == OP_SHR_SET || t == OP_SHL_SET;
}

// this code is horrible
void Var::mov(bool from_var, Gen &g) const
{
	Symbol &s = get();
	Gen::Size size = s.size;
	int val = s.offset_or_reg;
	bool reg = s.reg;
		
	const char *tmp;

	g.emit_mov(size);

	if (globl)
	{
		const std::string &name = s.name;

		tmp = new char[name.size() + 7];
		sprintf((char*)tmp, "%s(%%rip)", name.c_str());
	}
	else if (reg)
		tmp = Gen::REGS[size][Gen::SYSV_REGS[val]];
	else
	{
		if (val == 0)
			tmp = "(%rbp)";
		else
		{
			tmp = new char[std::to_string(val).size() + 7];
			sprintf((char*)tmp, "%d(%%rbp)", val);
		}
	}

	// get -> ax
	if (from_var)
	{
		g.append(tmp);
		g.append(", ");
	}

	if (reg) 
		g.append("%rax");
	else
		g.emit_reg(Gen::Reg::A, size);

	// ax -> get
	if (!from_var)
	{
		g.append(", ");
		g.append(tmp);
	}
		
	g.nl();

	if (globl || (!reg && val != 0))
		delete[] tmp;
}

// -------- grammars -------- //

// binop | assign
Expr *Parser::expr()
{
	TokType two_ahead = l.peek(2).type;
	if (is_assign(two_ahead))
	 	return assign();
	else
		return cond();
}

// [ expr ]
Expr *Parser::exp_option()
{
	if (l.pnxt().type == ';' || l.pnxt().type == ')')
	{
		Expr *tmp = new Expr;
		tmp->type = NONE;
		return tmp;
	}
	else
		return expr();
}

// 'return' expr ';' | if  | compound | exp_option ';' | KEY_BREAK ';' | KEY_CONT ';' | for | while | do
Node *Parser::stmt()
{
	Node *out = nullptr;
	bool semi = true;

	switch (l.pnxt().type) {
		case KEY_RETURN:
			l.eat(KEY_RETURN);
			out = new Ret(expr());
			break;
		case KEY_IF:
			out = if_stmt();
			semi = false;
			break;
		case KEY_FOR:
			out = for_stmt();
			semi = false;
			break;
		case KEY_WHILE:
			out = while_stmt();
			semi = false;
			break;
		case KEY_DO:
			out = do_stmt();
			semi = false;
			break;
		case KEY_BREAK:
			if (!in_loop)
				parse_err("Break cannot appear outside of loop body", l.pnxt());
			out = new Const(l.eat(KEY_BREAK));
			break;
		case KEY_CONT:
			if (!in_loop)
				parse_err("Continue cannot appear outside of loop body", l.pnxt());
			out = new Const(l.eat(KEY_CONT));
			break;
		case '{':
			out = compound();
			semi = false;
			break;
		case ')': 
			out = exp_option();
			semi = false;
			break;
		case ';': default:
			out = exp_option();
			break;
	}

	if (semi)
		l.eat((TokType)';');

	return out;
}

// params = [ type IDENTIFIER [ ',' params ] ]
// type IDENTIFIER '(' params ')' ( '{' { stmt | decl } '}' | ';' )
Stmt *Parser::func()
{
	bool prev_declared = false;

	Func *out = new Func;
	Scope *globl = scope;
	scope = new Scope(scope, scope->stack_index);

	Token tok = l.pnxt();
	TokType t = tok.type;
	if (!is_type(t))
		parse_err("Expected function return type", tok);

	// eat type
	l.eat(t);

	std::string name = std::get<std::string>(l.eat(IDENTIFIER).val);

	prev_declared = globl->in_scope(name);
		
	// get func name, add to sym tab
	globl->vec.push_back(Symbol(t, name, out, 0, false));

	out->name = Var(globl->vec.size() - 1, globl);

	l.eat((TokType)'(');

	// 8 for func ptr, 8 for rbp, -8 for the offset += 8
	int offset = 8;

	tok = l.pnxt();
	t = tok.type;
	while (t && t != ')')
	{
		if (!is_type(t))
			parse_err("Expected function param type", tok);
		
		l.eat(t);

		// get param name
		if (out->params.size() < 6)
			scope->vec.push_back(Symbol(
				t, std::get<std::string>(l.eat(IDENTIFIER).val),
				out, out->params.size(), true));
		else
			scope->vec.push_back(Symbol(
				t, std::get<std::string>(l.eat(IDENTIFIER).val),
				out, (offset += 8), false));

		out->params.push_back(Var(scope->vec.size() - 1, scope));

		if (l.pnxt().type != ')')
			l.eat((TokType)',');

		tok = l.pnxt();
		t = tok.type;
	}

	if (!t)
		parse_err("Unclosed parentheses in function definition", tok);
	
	if (prev_declared)
	{
		for (Symbol &s : globl->vec)
			// if a symbol is found with the same name
			// if the symbol is a function
			// if the function has different num of params than this function
			if (s.name == name)
			{
				if (s.node->type == FUNC && ((Func*)s.node)->params.size() != out->params.size())
					parse_err("Function parameter count does not match with previous declaration", tok);
				else if (s.node->type == DECL)
					parse_err("Redefition of variable " + name, tok);
			}
	}

	l.eat((TokType)')');

	if (l.pnxt().type == ';')
	{
		l.eat((TokType)';');
		out->blk = nullptr;
		scope = scope->parent_scope;
		return out;
	}

	// scope is reset in compound
	out->blk = compound(false);
	out->blk->func = true;

	return out;
}

// type IDENTIFIER [ '=' expr ] ';'
Decl *Parser::decl()
{
	bool globl = scope->parent_scope == nullptr;

	Token tok = l.pnxt();
	TokType t = tok.type;
	if (is_type(t))
		l.eat(t);
	else
		parse_err("Expected variable type preceding declaration", tok);

	std::string name = std::get<std::string>(l.eat(IDENTIFIER).val);

	// two calls to a looping search, not good
	if (scope->in_scope(name))
	{
		if (globl)
		{
			auto p = scope->get(name);
			if (p.second->vec[p.first].node->type == FUNC)
				parse_err("Redefinition of function " + name, tok);
		}

		parse_err("Redefinition of variable " + name, tok);
	}

	Decl *out = new Decl(Var(scope->vec.size(), scope, globl));

	if (globl)
		scope->vec.push_back(Symbol(t, name, out, 0, false));
	else
	{
		scope->add_var(t);
		scope->vec.push_back(Symbol(t, name, out, scope->stack_index, false));
	}

	if (l.pnxt().type == '=')
	{
		l.eat((TokType)'=');
		out->expr = expr();
	}

	l.eat((TokType)';');

	return out;
}

// 'if' '(' expr ')' ( stmt | blk ) [ 'else' ( stmt | blk) ]
Stmt *Parser::if_stmt()
{
	If *out = new If;

	l.eat(KEY_IF);
	l.eat((TokType)'(');

	out->cond = expr();

	l.eat((TokType)')');

	/*if (l.peek_next().type == '{')
		out->if_blk = blk();
	else*/
		out->if_blk = stmt();

	if (l.pnxt().type == KEY_ELSE)
	{
		l.eat(KEY_ELSE);

		/*if (l.peek_next().type == '{')
			out->else_blk = blk();
		else*/
			out->else_blk = stmt();
	}

	return out;
}

// KEY_FOR '(' ( exp_option ';' | decl ) exp_option ';' exp_option ')' stmt
Stmt *Parser::for_stmt()
{
	l.eat(KEY_FOR);
	l.eat((TokType)'(');

	// parse decl
	if (is_type(l.pnxt().type))
	{
		ForDecl *out = new ForDecl;

		scope = new Scope(scope, scope->stack_index);
		out->for_scope = scope;

		out->init = decl();
		out->cond = exp_option();
		l.eat((TokType)';');
		out->post = exp_option();
		l.eat((TokType)')');

		in_loop = true;
		out->blk = stmt();
		in_loop = false;

		scope = scope->parent_scope;

		return out;
	}
	else
	{
		For *out = new For;
		out->init = exp_option();
		l.eat((TokType)';');
		out->cond = exp_option();
		l.eat((TokType)';');
		out->post = exp_option();
		l.eat((TokType)')');

		in_loop = true;
		out->blk = stmt();
		in_loop = false;

		return out;
	}
}

// KEY_WHILE '(' expr ')' stmt
Stmt *Parser::while_stmt()
{
	While *out = new While;

	l.eat(KEY_WHILE);
	l.eat((TokType)'(');

	out->cond = expr();

	l.eat((TokType)')');

	in_loop = true;
	out->blk = stmt();
	in_loop = false;

	return out;
}

// KEY_DO stmt KEY_WHILE '(' expr ')' ';'
Stmt *Parser::do_stmt()
{
	Do *out = new Do;

	l.eat(KEY_DO);

	in_loop = true;
	out->blk = stmt();
	in_loop = false;

	l.eat(KEY_WHILE);
	l.eat((TokType)'(');

	out->cond = expr();

	l.eat((TokType)')');
	l.eat((TokType)';');

	return out;
}

// '{' { stmt | decl } '}'
Compound *Parser::compound(bool newscope)
{
	Compound *out;

	if (newscope)
	 	out = new Compound((scope = new Scope(scope, scope->stack_index)));
	else
	 	out = new Compound(scope);

	l.eat((TokType)'{');

	Token tok = l.pnxt();
	TokType t = tok.type;
	while (t && t != '}')
	{
		if (is_type(t))
			out->vec.push_back(decl());
		else
			out->vec.push_back(stmt());
		
		tok = l.pnxt();
		t = tok.type;
	}

	if (!t)
		parse_err("Unterminated function body", tok);

	l.eat((TokType)'}');

	// reset scope
	scope = scope->parent_scope;

	return out;
}

// -------- expressions -------- //

// IDENTIFIER
Expr *Parser::lval()
{
	auto p = scope->get(std::get<std::string>(l.eat(IDENTIFIER).val));
	return new Var(p.first, p.second, ((Decl*)p.second->vec[p.first].node)->v.globl);
}

// lval assign expr
Expr *Parser::assign()
{
	Expr *lv = lval();

	Token tok = l.pnxt();
	TokType t = tok.type;
	if (!is_assign(t))
		parse_err("Expected assignment operator", tok);
	
	l.eat(t);

	return new Assign(t, lv, expr());
}

// -------- binary operations -------- //

#define BINEXP(func, call_func, type_eval)  	  \
	Expr *Parser::func()						  \
	{											  \
		Expr *out = call_func();				  \
		TokType t = l.pnxt().type;				  \
												  \
		while (type_eval)						  \
		{										  \
			l.eat(t);							  \
			out = new BinOp(t, out, call_func()); \
			t = l.pnxt().type;					  \
		}										  \
												  \
		return out;								  \
	}

// op_or [ '?' expr ':' cond ]
Expr *Parser::cond()
{
	Expr *c = op_or();

	if (l.pnxt().type == '?')
	{
		l.eat((TokType)'?');

		Expr *t = expr();

		l.eat((TokType)':');

		return new Cond(c, t, cond());
	}
	else
		return c;
}

// op_and { '||' op_and }
BINEXP(op_or, op_and, t == OP_OR)

// bitwise_or { '&&' bitwise_or }
BINEXP(op_and, bitwise_or, t == OP_AND)

// bitwise_xor { '|' bitwise_xor }
BINEXP(bitwise_or, bitwise_xor, t == '|')

// bitwise_and { '^' bitwise_and }
BINEXP(bitwise_xor, bitwise_and, t == '^')

// equality { '&' equality }
BINEXP(bitwise_and, equality, t == '&')

// comparison { ( OP_EQ | OP_NE ) comparison }
BINEXP(equality, comparison, t == OP_EQ || t == OP_NE)

// shift { ( OP_LE | OP_GE | '<' | '>' ) shift }
BINEXP(comparison, shift, t == OP_LE || t == OP_GE || t == '<' || t == '>')

// term { ( OP_SHR | OP_SHL ) term }
BINEXP(shift, term, t == OP_SHL || t == OP_SHR)

// factor { ( '+' | '-' ) factor }
BINEXP(term, factor, t == '+' || t == '-')

// unop { ( '/' | '*' | '%' ) unop }
BINEXP(factor, unop, t == '/' || t == '*' || t == '%')

// postfix | ( OP_INC | OP_DEC ) lval | ( '!' | '~' | '-' ) unop
Expr *Parser::unop()
{
	TokType t = l.pnxt().type;
	if (t == OP_INC || t == OP_DEC)
	{
		l.eat(t);

		return new UnOp(t, lval());
	}
	else if (t == '!' || t == '~' || t == '-')
	{
		l.eat(t);

		return new UnOp(t, unop());
	}
	else
		return postfix();
}

// primary [ OP_INC | OP_DEC ]
Expr *Parser::postfix()
{
	Expr *p = primary();

	TokType t = l.pnxt().type;
	if (t == OP_INC || t == OP_DEC)
	{
		l.eat(t);
		return new Post(t, p);
	}
	else
		return p;
}

// call | INT_CONSTANT | FP_CONSTANT | STR_CONSTANT | CHAR_CONSTANT | IDENTIFIER | '(' expr ')'
Expr *Parser::primary()
{
	Token tok = l.pnxt();
	TokType t = tok.type;

	if (t == INT_CONSTANT || t == FP_CONSTANT || t == STR_CONSTANT || t == CHAR_CONSTANT)
	{
		l.eat(t);
		return new Const(tok);
	}
	else if (t == IDENTIFIER)
	{
		if (l.peek(2).type == '(')
			return call();
		else
		{
			auto p = scope->get(std::get<std::string>(l.eat(IDENTIFIER).val));
			return new Var(p.first, p.second, ((Decl*)p.second->vec[p.first].node)->v.globl);
		}
	}
	else if (t == '(')
	{
		l.eat((TokType)('('));
		Expr *out = expr();
		l.eat((TokType)(')'));

		return out;
	}

	parse_err(std::string("Invalid expression ") + l.getname(t), tok);
	return nullptr;
}

// IDENTIFIER '(' [ expr { ',' expr } ] ')'
Expr *Parser::call()
{
	Call *out = new Call;

	// get function from scope
	auto p = scope->get(std::get<std::string>(l.eat(IDENTIFIER).val));
	out->func = Var(p.first, p.second);

	l.eat((TokType)'(');

	Token tok = l.pnxt();
	TokType t = tok.type;
	while (t && t != ')')
	{
		out->params.push_back(expr());

		if (l.pnxt().type != ')')
			l.eat((TokType)',');
		
		tok = l.pnxt();
		t = tok.type;
	}

	if (!t)
		parse_err("Unclosed parentheses in function call", tok);
	
	if (p.second->vec[p.first].node->type != FUNC)
		parse_err("Attempting to call variable", tok);
	if (out->params.size() != ((Func*)p.second->vec[p.first].node)->params.size())
		parse_err("Too many arguments to function call", tok);

	l.eat((TokType)')');

	return out;
}

// -------- main program blk -------- //

// statementlist = { func | decl }
Compound *Parser::parse()
{
	Compound *out = new Compound(scope);

	TokType t = l.peek(3).type;
	while (l.pnxt().type)
	{
		if (t)
		{
			if (t == '(')
				out->vec.push_back(func());
			else
				out->vec.push_back(decl());
		}

		t = l.peek(3).type;
	}
	
	return out;
}

void Parser::parse_err(const std::string &msg, const Token &err_tok)
{
	std::cerr << msg
			  << " at line " << err_tok.line
			  << ", col " << err_tok.col
			  << '\n';

	exit(1);
}
