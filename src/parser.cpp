#include <parser.hpp>

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
AST *Parser::expr()
{
	TokType two_ahead = l.peek(2).type;
	if (is_assign(two_ahead))
	 	return assign();
	else
		return cond();
}

// [ expr ]
AST *Parser::exp_option()
{
	if (l.pnxt().type == ';' || l.pnxt().type == ')')
		return new AST(NONE);
	else
		return expr();
}

// 'return' expr ';' | if  | compound | exp_option ';' | KEY_BREAK ';' | KEY_CONT ';' | for | while | do
AST *Parser::stmt()
{
	AST *out;
	bool semi = true;

	switch (l.pnxt().type) {
		case KEY_RETURN:
			l.eat(KEY_RETURN);
			out = new AST(RET, expr());
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
			l.eat(KEY_BREAK);
			out = new AST(BREAK);
			break;
		case KEY_CONT:
			if (!in_loop)
				parse_err("Continue cannot appear outside of loop body", l.pnxt());
			l.eat(KEY_CONT);
			out = new AST(CONT);
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
		l.eat(static_cast<TokType>(';'));

	return out;
}

// params = [ type IDENTIFIER [ ',' params ] ]
// type IDENTIFIER '(' params ')' ( '{' { stmt | decl } '}' | ';' )
// AST - lhs = symtab entry, mid = params, rhs = block
AST *Parser::func()
{
	AST *out = new AST(FUNC);

	Token tok = l.pnxt();
	TokType t = tok.type;
	if (!is_type(t))
		parse_err("Expected function return type", tok);

	l.eat(t);

	// symbol entry //

	std::string name = std::get<std::string>(l.eat(IDENTIFIER).val);

	Scope *globl = Scope::s(Scope::GLOBAL);
	Scope *cur = Scope::s(cur_scope);
	bool prev_declared = globl->in_scope(name);

	// add to sym tab
	globl->syms.push_back(Sym(V_FUNC, t, name, 0));

	out->lhs = new AST(VAR, globl->syms.size() - 1, Scope::GLOBAL);

	l.eat(static_cast<TokType>('('));
	
	// parameters //

	// 8 for func ptr, 8 for rbp, -8 for the offset += 8
	int offset = 8;
	int param_count = 0;
	cur_scope = Scope::new_scope(cur_scope);
	AST *bottom = out->mid;

	tok = l.pnxt();
	t = tok.type;
	while (t && t != ')')
	{
		if (!is_type(t))
			parse_err("Expected function param type", tok);
		
		l.eat(t);

		// get param name
		if (param_count < 6)
			cur->syms.push_back(Sym(V_REG, t,
				std::get<std::string>(l.eat(IDENTIFIER).val),
				ARG_START + param_count));
		else
			cur->syms.push_back(Sym(V_VAR, t,
				std::get<std::string>(l.eat(IDENTIFIER).val),
				offset += 8));

		bottom = AST::append(bottom, new AST(VAR, cur->syms.size() - 1, cur_scope), PARAM);

		if (l.pnxt().type != ')')
			l.eat(static_cast<TokType>(','));

		tok = l.pnxt();
		t = tok.type;
	}

	if (!t)
		parse_err("Unclosed parentheses in function definition", tok);
	
	// set value to param count
	globl->syms.back().val = param_count;
	
	if (prev_declared)
	{
		for (Sym &s : globl->syms)
			// if a symbol is found with the same name
			if (s.name == name)
			{
				// if the symbol is a function
				// if the function has different num of params than this function
				if (s.vtype == V_FUNC && s.val != param_count)
					parse_err("Function parameter count does not match with previous declaration", tok);
				else if (s.vtype == V_GLOBL)
					parse_err("Redefition of variable " + name, tok);
			}
	}

	l.eat(static_cast<TokType>(')'));

	if (l.pnxt().type == ';')
	{
		l.eat(static_cast<TokType>(';'));
		cur_scope = cur->parent_id;
		return out;
	}

	// scope is reset in compound
	offset = 0;
	out->rhs = compound(false);

	return out;
}

// type IDENTIFIER [ '=' expr ] ';'
AST *Parser::decl()
{
	bool globl = cur_scope == 0;

	Token tok = l.pnxt();
	TokType type = tok.type;
	if (is_type(type))
		l.eat(type);
	else
		parse_err("Expected variable type preceding declaration", tok);

	Token id = l.eat(IDENTIFIER);
	std::string name = std::get<std::string>(id.val);

	AST *out = new AST(DECL, new AST(VAR, Scope::s(cur_scope)->syms.size(), cur_scope));

	if (globl)
		Scope::s(cur_scope)->syms.push_back(Sym(V_GLOBL, type, name));
	else
		Scope::s(cur_scope)->syms.push_back(Sym(V_VAR, type, name, offset += getsize(type)));

	if (l.pnxt().type == '=')
	{
		Scope::s(cur_scope)->syms.back().val = true;

		out->op = static_cast<TokType>('=');
		l.eat(static_cast<TokType>('='));
		out->rhs = expr();
	}

	// two calls to a linear search, not good
	if (Scope::s(cur_scope)->in_scope(name))
	{
		if (globl)
		{
			auto p = Scope::s(cur_scope)->get(name);
			Sym &s = Scope::s(p.second)->syms[p.first];
			if (s.vtype == V_FUNC)
				parse_err("Redefinition of function " + name, id);
			else if (out->rhs && s.vtype == V_GLOBL && s.val)
				parse_err("Redefinition of variable " + name, id);
		}
		else
			parse_err("Redefinition of variable " + name, id);
	}

	l.eat(static_cast<TokType>(';'));

	return out;
}

// 'if' '(' expr ')' ( stmt | blk ) [ 'else' ( stmt | blk) ]
// lhs = cond, mid = if, rhs = else
AST *Parser::if_stmt()
{
	AST *out = new AST(IF);

	l.eat(KEY_IF);
	l.eat(static_cast<TokType>('('));

	out->lhs = expr();

	l.eat(static_cast<TokType>(')'));

	out->mid = stmt();

	if (l.pnxt().type == KEY_ELSE)
	{
		l.eat(KEY_ELSE);

		out->rhs = stmt();
	}

	return out;
}

// KEY_FOR '(' ( exp_option ';' | decl ) exp_option ';' exp_option ')' stmt
// lhs = init, mid = cond, rhs->lhs = compound, rhs->rhs = post
AST *Parser::for_stmt()
{
	l.eat(KEY_FOR);
	l.eat(static_cast<TokType>('('));
	
	AST *out = new AST();
	AST *child = new AST(FOR);
	out->rhs = child;

	bool declaration = is_type(l.pnxt().type);

	out->type = declaration ? FOR_DECL : FOR;

	// parse decl
	if (declaration)
	{
		cur_scope = Scope::new_scope(cur_scope);
		out->val = cur_scope;

		out->lhs = decl();
	}
	else
	{
		out->lhs = exp_option();
		l.eat(static_cast<TokType>(';'));
	}

	out->mid = exp_option();
	l.eat(static_cast<TokType>(';'));
	child->rhs = exp_option();
	l.eat(static_cast<TokType>(')'));

	in_loop = true;
	child->lhs = stmt();

	if (declaration)
		cur_scope = Scope::s(cur_scope)->parent_id;

	in_loop = false;

	return out;
}

// KEY_WHILE '(' expr ')' stmt
// lhs = cond, rhs = blk
AST *Parser::while_stmt()
{
	AST *out = new AST(WHILE);

	l.eat(KEY_WHILE);
	l.eat(static_cast<TokType>('('));

	out->lhs = expr();

	l.eat(static_cast<TokType>(')'));

	in_loop = true;
	out->rhs = stmt();
	in_loop = false;

	return out;
}

// KEY_DO stmt KEY_WHILE '(' expr ')' ';'
// lhs = cond, rhs = blk
AST *Parser::do_stmt()
{
	AST *out = new AST(DO);

	l.eat(KEY_DO);

	in_loop = true;
	out->rhs = stmt();
	in_loop = false;

	l.eat(KEY_WHILE);
	l.eat(static_cast<TokType>('('));

	out->lhs = expr();

	l.eat(static_cast<TokType>(')'));
	l.eat(static_cast<TokType>(';'));

	return out;
}

// '{' { stmt | decl } '}'
// block stored in lhs
AST *Parser::compound(bool newscope)
{
	if (newscope)
		cur_scope = Scope::new_scope(cur_scope);
	
	AST *out = new AST(BLOCK, cur_scope);
	AST *bottom = out->lhs;

	l.eat(static_cast<TokType>('{'));

	Token tok = l.pnxt();
	TokType t = tok.type;
	while (t && t != '}')
	{
		if (is_type(t))
			bottom = AST::append(bottom, decl(), BLOCK);
		else
			bottom = AST::append(bottom, stmt(), BLOCK);
		
		tok = l.pnxt();
		t = tok.type;
	}

	if (!t)
		parse_err("Unterminated function body", tok);

	l.eat(static_cast<TokType>('}'));

	// reset scope
	cur_scope = Scope::s(cur_scope)->parent_id;

	return out;
}

// -------- expressions -------- //

// IDENTIFIER
AST *Parser::lval()
{
	auto p = Scope::s(cur_scope)->get(std::get<std::string>(l.eat(IDENTIFIER).val));
	return new AST(VAR, p.first, p.second);
}

// lval assign expr
// lhs t rhs
AST *Parser::assign()
{
	AST *lv = lval();

	Token tok = l.pnxt();
	TokType t = tok.type;
	if (!is_assign(t))
		parse_err("Expected assignment operator", tok);
	
	l.eat(t);

	return new AST(ASSIGN, t, lv, expr());
}

// -------- binary operations -------- //

#define BINEXP(func, call_func, type_eval)             \
	AST *Parser::func()                                \
	{                                                  \
		AST *out = call_func();                        \
		TokType t = l.pnxt().type;                     \
                                                       \
		while (type_eval)                              \
		{                                              \
			l.eat(t);                                  \
			out = new AST(BINOP, t, out, call_func()); \
			t = l.pnxt().type;                         \
		}                                              \
                                                       \
		return out;                                    \
	}

// op_or [ '?' expr ':' cond ]
AST *Parser::cond()
{
	AST *c = op_or();

	if (l.pnxt().type == '?')
	{
		l.eat(static_cast<TokType>('?'));

		AST *t = expr();

		l.eat(static_cast<TokType>(':'));

		return new AST(COND, c, t, cond());
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

// postfix | ( OP_INC | OP_DEC | '&' | '* ) lval | ( '!' | '~' | '-' ) unop
// lhs = operand
AST *Parser::unop()
{
	TokType t = l.pnxt().type;
	if (t == OP_INC || t == OP_DEC || t == '&' || t == '*')
	{
		l.eat(t);

		return new AST(UNOP, t, lval());
	}
	else if (t == '!' || t == '~' || t == '-')
	{
		l.eat(t);

		return new AST(UNOP, t, unop());
	}
	else
		return postfix();
}

// primary [ OP_INC | OP_DEC ]
AST *Parser::postfix()
{
	AST *p = primary();

	TokType t = l.pnxt().type;
	if (t == OP_INC || t == OP_DEC)
	{
		l.eat(t);
		return new AST(POSTFIX, t, p);
	}
	else
		return p;
}

// call | INT_CONSTANT | FP_CONSTANT | STR_CONSTANT | CHAR_CONSTANT | IDENTIFIER | '(' expr ')'
AST *Parser::primary()
{
	Token tok = l.pnxt();
	TokType t = tok.type;

	// if (t == INT_CONSTANT || t == FP_CONSTANT || t == STR_CONSTANT || t == CHAR_CONSTANT)
	if (t == INT_CONSTANT || t == CHAR_CONSTANT)
	{
		l.eat(t);
		return new AST(CONST, std::get<long long>(tok.val));
	}
	else if (t == IDENTIFIER)
	{
		if (l.peek(2).type == '(')
			return call();
		else
		{
			auto p = Scope::s(cur_scope)->get(std::get<std::string>(l.eat(IDENTIFIER).val));
			// scope entry and scope id
			return new AST(VAR, p.first, p.second);
		}
	}
	else if (t == '(')
	{
		l.eat(static_cast<TokType>(('(')));
		AST *out = expr();
		l.eat(static_cast<TokType>((')')));

		return out;
	}

	parse_err(std::string("Invalid expression ") + l.getname(t), tok);
}

// IDENTIFIER '(' [ expr { ',' expr } ] ')'
AST *Parser::call()
{
	AST *out = new AST(CALL);

	// get symbol from scope
	Token id = l.eat(IDENTIFIER);

	auto p = Scope::s(cur_scope)->get(std::get<std::string>(id.val));
	out->lhs = new AST(VAR, p.first, p.second);

	if (Scope::s(p.second)->syms[p.first].vtype != V_FUNC)
		parse_err("Attempting to call variable", id);

	l.eat(static_cast<TokType>('('));

	int param_count = 0;
	AST *list_bottom = out->rhs;

	Token tok = l.pnxt();
	TokType t = tok.type;
	while (t && t != ')')
	{
		list_bottom = AST::append(list_bottom, expr(), LIST);
		++param_count;

		if (l.pnxt().type != ')')
			l.eat(static_cast<TokType>(','));
		
		tok = l.pnxt();
		t = tok.type;
	}

	if (!t)
		parse_err("Unclosed parentheses in function call", tok);
	
	// TODO: add back in checking for param count
	// if (param_count != ((Func*)p.second->vec[p.first].node)->params.size())
	// 	parse_err("Too many arguments to function call", tok);

	l.eat(static_cast<TokType>(')'));

	return out;
}

// -------- main program blk -------- //

// statementlist = { func | decl }
AST *Parser::parse()
{
	AST *out = new AST(BLOCK);
	AST *bottom = out;

	TokType t = l.peek(3).type;

	while (l.pnxt().type)
	{
		if (t == '(')
			bottom = AST::append(bottom, func(), BLOCK);
		else
			bottom = AST::append(bottom, decl(), BLOCK);

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
