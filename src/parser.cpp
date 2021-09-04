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

// -------- grammars -------- //

// binop | assign
Expr *Parser::expr()
{
	if (is_assign(l.peek(2).type))
	 	return assign();
	else
		return cond();
}

// 'return' expr ';' | expr ';' | if  | compound
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
		case '{':
			out = compound();
			semi = false;
			break;
		default:
			out = expr();
			break;
	}

	if (semi)
		l.eat((TokType)';');

	return out;
}

// params = [ ( type IDENTIFIER ) | ( type IDENTIFIER ',' params ) ]
// type IDENTIFIER '(' params ')' '{' { stmt | decl } '}'
Stmt *Parser::func()
{
	Func *out = new Func;
	scope = new Scope(scope, scope->stack_index);

	Token tok = l.pnxt();
	TokType t = tok.type;
	if (!is_type(t))
		parse_err("Expected function return type", tok);

	// eat type
	l.eat(t);
		
	// get func name, add to sym tab
	scope->vec.emplace_back(Symbol(t, std::get<std::string>(l.eat(IDENTIFIER).val)));

	out->name = Var(scope->vec.size() - 1, scope);

	l.eat((TokType)'(');

	while (l.pnxt().type != ')')
	{
		tok = l.pnxt();
		t = tok.type;

		if (!is_type(t) && t != KEY_VOID)
			parse_err("Expected function param type", tok);
		
		l.eat(t);

		// get param name
		out->params.push_back(Symbol(t, std::get<std::string>(l.eat(IDENTIFIER).val)));

		if (l.pnxt().type != ')')
			l.eat((TokType)',');
	}

	l.eat((TokType)')');

	// scope is reset in compound
	out->blk = compound();
	out->blk->func = true;

	return out;
}

// int IDENTIFIER [ '=' expr ] ';'
Stmt *Parser::decl()
{
	Token tok = l.pnxt();
	TokType t = tok.type;
	l.eat(KEY_INT);

	std::string name = std::get<std::string>(l.eat(IDENTIFIER).val);

	for (Symbol &s : scope->vec)
	{
		if (s.name == name)
			parse_err("Redefinition of variable " + name, tok);
	}

	scope->vec.push_back(Symbol(t, name));

	// only 32 bit int for now
	scope->vec.back().bp_offset = (scope->stack_index += 4);

	Decl *out = new Decl(Var(scope->vec.size() - 1, scope));

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

// '{' { stmt | decl } '}'
Compound *Parser::compound(bool newscope)
{
	Compound *out;

	if (newscope)
	 	out = new Compound(scope = new Scope(scope, scope->stack_index));
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
	return new Var(p.first, p.second);
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

// INT_CONSTANT | FP_CONSTANT | STR_CONSTANT | IDENTIFIER | '(' expr ')'
Expr *Parser::primary()
{
	Token tok = l.pnxt();
	TokType t = tok.type;

	if (t == INT_CONSTANT || t == FP_CONSTANT || t == STR_CONSTANT)
	{
		l.eat(t);
		return new Const(tok);
	}
	else if (t == IDENTIFIER)
	{
		auto p = scope->get(std::get<std::string>(l.eat(IDENTIFIER).val));
		return new Var(p.first, p.second);
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

// -------- main program blk -------- //

// statementlist = function statementlist | function EOF
Compound *Parser::parse()
{
	Compound *out = new Compound(scope);

	while (l.pnxt().type)
		out->vec.push_back(func());
	
	return out;
}

void Parser::parse_err(const std::string &msg, const Token &err_tok)
{
	std::cerr << "Error: " << msg
			  << " at line " << err_tok.line
			  << ", col " << err_tok.col
			  << '\n';

	exit(1);
}
