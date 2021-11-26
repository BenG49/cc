#include <parser.hpp>

#include <codegen.hpp>
#include <types.hpp>
#include <err.hpp>

static bool is_type(TokType t)
{
	return t == KEY_BOOL || t == KEY_CHAR || t == KEY_INT || t == KEY_FLOAT || t == KEY_VOID || t == KEY_LONG;
}

static bool is_assign(TokType t)
{
	return t == OP_SET || t == OP_ADD_SET || t == OP_SUB_SET || t == OP_MUL_SET
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
	if (l.pnxt().type == SEMI || l.pnxt().type == RPAREN)
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
			out = new AST(RET, INT, exp_option());
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
				err_tok("Break cannot appear outside of loop body", l.pnxt());
			l.eat(KEY_BREAK);
			out = new AST(BREAK);
			break;
		case KEY_CONT:
			if (!in_loop)
				err_tok("Continue cannot appear outside of loop body", l.pnxt());
			l.eat(KEY_CONT);
			out = new AST(CONT);
			break;
		case LBRAC:
			out = compound();
			semi = false;
			break;
		case RPAREN: 
			out = exp_option();
			semi = false;
			break;
		case SEMI: default:
			out = exp_option();
			break;
	}

	if (semi)
		l.eat(SEMI);

	return out;
}

// params = [ type IDENTIFIER [ ',' params ] ]
// type IDENTIFIER '(' params ')' ( '{' { stmt | decl } '}' | ';' )
// AST - lhs = symtab entry, mid = params, rhs = block
AST *Parser::func()
{
	AST *out = new AST(FUNC);
	out->mid = new AST(LIST);

	Token tok = l.pnxt();
	TokType t = tok.type;
	if (!is_type(t))
		err_tok("Expected function return type", tok);

	l.eat(t);
	PrimType p = Parser::asptype(t);

	// symbol entry //

	std::string name = std::get<std::string>(l.eat(IDENTIFIER).val);

	Scope *globl = Scope::s(Scope::GLOBAL);
	bool prev_declared = globl->in_scope(name);

	// add to sym tab
	globl->syms.push_back(Sym(V_FUNC, p, name, 0));
	out->lhs = new AST(p, globl->syms.size() - 1, Scope::GLOBAL);

	l.eat(LPAREN);
	
	// parameters //

	// 8 for func ptr, 8 for rbp, -8 for the offset += 8
	int p_offset = 8;
	int param_count = 0;
	cur_scope = Scope::new_scope(cur_scope);
	Scope *cur = Scope::s(cur_scope);
	AST *bottom = out->mid;

	tok = l.pnxt();
	t = tok.type;
	while (t != RPAREN)
	{
		if (!is_type(t))
			err_tok("Expected function param type", tok);
		
		l.eat(t);
		PrimType p = Parser::asptype(t);

		// get param name
		if (param_count < ARG_COUNT)
			cur->syms.push_back(Sym(V_REG, p,
				std::get<std::string>(l.eat(IDENTIFIER).val),
				FIRST_ARG + param_count));
		else
			cur->syms.push_back(Sym(V_VAR, p,
				std::get<std::string>(l.eat(IDENTIFIER).val),
				p_offset += 8));

		bottom = AST::append(bottom, new AST(p, cur->syms.size() - 1, cur_scope), LIST);

		++param_count;

		if (l.pnxt().type != RPAREN)
			l.eat(COMMA);

		tok = l.pnxt();
		t = tok.type;
	}

	if (!t)
		err_tok("Unclosed parentheses in function definition", tok);
	
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
					err_tok("Function parameter count does not match with previous declaration", tok);
				else if (s.vtype == V_GLOBL)
					err_tok("Redefition of variable " + name, tok);
			}
	}

	l.eat(RPAREN);

	if (l.pnxt().type == SEMI)
	{
		l.eat(SEMI);
		cur_scope = cur->parent_id;
		return out;
	}

	// scope is reset in compound
	offset = 0;
	out->rhs = compound(false);
	out->val = offset;

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
		err_tok("Expected variable type preceding declaration", tok);

	Token id = l.eat(IDENTIFIER);
	PrimType t = Parser::asptype(id.type);
	std::string name = std::get<std::string>(id.val);

	AST *out = new AST(DECL, t, new AST(t, Scope::s(cur_scope)->syms.size(), cur_scope));

	bool assigned = l.pnxt().type == OP_SET;

	// two calls to a linear search, not good
	if (Scope::s(cur_scope)->in_scope(name))
	{
		if (globl)
		{
			Sym &s = Scope::s(cur_scope)->get(name)->get_sym();
			if (s.vtype == V_FUNC)
				err_tok("Redefinition of function " + name, id);
			else if (assigned && s.vtype == V_GLOBL && s.val)
				err_tok("Redefinition of variable " + name, id);
		}
		else
			err_tok("Redefinition of variable " + name, id);
	}

	if (globl)
		Scope::s(cur_scope)->syms.push_back(Sym(V_GLOBL, Parser::asptype(type), name));
	else
	{
		int sz = 1 << getsize(type);
		Scope::s(cur_scope)->syms.push_back(Sym(V_VAR, Parser::asptype(type), name, (offset -= sz)));
		stk_size += sz;
	}

	if (assigned)
	{
		// use val to store if assigned or not
		if (globl)
			Scope::s(cur_scope)->syms.back().val = true;

		out->type = DECL_SET;
		l.eat(OP_SET);
		out->rhs = expr();

		// hacky fix for setting char to integer constant
		if (out->lhs->get_sym().type == CHAR && out->rhs->type == INT_CONST)
			out->rhs->ptype = CHAR;
	}

	l.eat(SEMI);

	return out;
}

// 'if' '(' expr ')' ( stmt | blk ) [ 'else' ( stmt | blk) ]
// lhs = cond, mid = if, rhs = else
AST *Parser::if_stmt()
{
	AST *out = new AST(IF);

	l.eat(KEY_IF);
	l.eat(LPAREN);

	out->lhs = expr();

	l.eat(RPAREN);

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
	l.eat(LPAREN);
	
	AST *out = new AST();
	AST *child = new AST(FOR);
	out->rhs = child;

	bool declaration = is_type(l.pnxt().type);

	out->type = declaration ? FOR_DECL : FOR;

	// parse decl
	if (declaration)
	{
		cur_scope = Scope::new_scope(cur_scope);
		out->scope_id = cur_scope;

		// kinda stupid, saves stack size
		int tmp = stk_size;
		out->lhs = decl();
		out->val = stk_size - tmp;
		stk_size = tmp;
	}
	else
	{
		out->lhs = exp_option();
		l.eat(SEMI);
	}

	out->mid = exp_option();
	l.eat(SEMI);
	child->rhs = exp_option();
	l.eat(RPAREN);

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
	l.eat(LPAREN);

	out->lhs = expr();

	l.eat(RPAREN);

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
	l.eat(LPAREN);

	out->lhs = expr();

	l.eat(RPAREN);
	l.eat(SEMI);

	return out;
}

// '{' { stmt | decl } '}'
// block stored in lhs, size stored in val
AST *Parser::compound(bool newscope)
{
	stk_size = 0;

	if (newscope)
		cur_scope = Scope::new_scope(cur_scope);
	
	AST *out = new AST(LIST);
	out->scope_id = cur_scope;
	AST *bottom = out;

	l.eat(LBRAC);

	Token tok = l.pnxt();
	TokType t = tok.type;
	while (t != RBRAC)
	{
		if (is_type(t))
			bottom = AST::append(bottom, decl(), LIST);
		else
			bottom = AST::append(bottom, stmt(), LIST);
		
		tok = l.pnxt();
		t = tok.type;
	}

	if (!t)
		err_tok("Unterminated function body", tok);

	l.eat(RBRAC);

	// reset scope
	cur_scope = Scope::s(cur_scope)->parent_id;

	// aka not a function
	if (newscope)
		out->val = stk_size;

	return out;
}

// -------- expressions -------- //

// IDENTIFIER
AST *Parser::lval()
{
	return Scope::s(cur_scope)->get(std::get<std::string>(l.eat(IDENTIFIER).val));
}

// lval assign expr
// lhs t rhs
AST *Parser::assign()
{
	AST *lv = lval();

	Token tok = l.pnxt();
	TokType t = tok.type;
	if (!is_assign(t))
		err_tok("Expected assignment operator", tok);
	
	l.eat(t);

	AST *rhs = expr();

	// hacky fix for setting char to integer constant
	if (lv->get_sym().type == CHAR && rhs->type == INT_CONST)
		rhs->ptype = CHAR;

	return new AST(Parser::asnode(t), INT, lv, rhs);
}

// -------- binary operations -------- //

#define BINEXP(func, call_func, type_eval, node)                                               \
	AST *Parser::func()                                                                        \
	{                                                                                          \
		AST *out = call_func();                                                                \
		TokType t = l.pnxt().type;                                                             \
                                                                                               \
		while (type_eval)                                                                      \
		{                                                                                      \
			l.eat(t);                                                                          \
			AST *c = call_func();                                                              \
			out = new AST(node,                                                                \
						  (p_sizeof(out->ptype) > p_sizeof(c->ptype)) ? out->ptype : c->ptype, \
						  out, c);                                                             \
			t = l.pnxt().type;                                                                 \
		}                                                                                      \
                                                                                               \
		return out;                                                                            \
	}

#define BINEXP_ASNODE(func, call_func, type_eval)                                              \
	AST *Parser::func()                                                                        \
	{                                                                                          \
		AST *out = call_func();                                                                \
		TokType t = l.pnxt().type;                                                             \
                                                                                               \
		while (type_eval)                                                                      \
		{                                                                                      \
			l.eat(t);                                                                          \
			AST *c = call_func();                                                              \
			out = new AST(Parser::asnode(t),                                                   \
						  (p_sizeof(out->ptype) > p_sizeof(c->ptype)) ? out->ptype : c->ptype, \
						  out, c);                                                             \
			t = l.pnxt().type;                                                                 \
		}                                                                                      \
                                                                                               \
		return out;                                                                            \
	}

// op_or [ '?' expr ':' cond ]
AST *Parser::cond()
{
	AST *c = op_or();

	if (l.pnxt().type == OP_COND)
	{
		l.eat(OP_COND);

		AST *t = expr();

		l.eat(OP_COLON);

		return new AST(COND, INT, c, t, cond());
	}
	else
		return c;
}

// op_and { '||' op_and }
BINEXP(op_or, op_and, t == OP_LOGOR, LOGOR)

// bitwise_or { '&&' bitwise_or }
BINEXP(op_and, bitwise_or, t == OP_LOGAND, LOGAND)

// bitwise_xor { '|' bitwise_xor }
BINEXP(bitwise_or, bitwise_xor, t == OP_OR, OR)

// bitwise_and { '^' bitwise_and }
BINEXP(bitwise_xor, bitwise_and, t == OP_XOR, XOR)

// equality { '&' equality }
BINEXP(bitwise_and, equality, t == OP_AMPER, AND)

// comparison { ( OP_EQ | OP_NE ) comparison }
BINEXP_ASNODE(equality, comparison, t == OP_EQ || t == OP_NE)

// shift { ( OP_LE | OP_GE | '<' | '>' ) shift }
BINEXP_ASNODE(comparison, shift, t == OP_LE || t == OP_GE || t == OP_LT || t == OP_GT)

// term { ( OP_SHR | OP_SHL ) term }
BINEXP_ASNODE(shift, term, t == OP_SHL || t == OP_SHR)

// factor { ( '+' | '-' ) factor }
BINEXP_ASNODE(term, factor, t == OP_ADD || t == OP_SUB)

// unop { ( '/' | '*' | '%' ) unop }
BINEXP_ASNODE(factor, unop, t == OP_DIV || t == OP_MUL || t == OP_MOD)

// postfix | ( OP_INC | OP_DEC | '&' | '* ) lval | ( '!' | '~' | '-' ) unop
// lhs = operand
AST *Parser::unop()
{
	TokType t = l.pnxt().type;

	NodeType n;
	bool lv = true;

	switch (t) {
		case OP_INC: n = UN_INC; break;
		case OP_DEC: n = UN_DEC; break;
		case OP_AMPER: n = REF; break;
		case OP_MUL: n = PTR; break;
		case OP_SUB:
			n = NEG; lv = false; break;
		case OP_LOGNOT:
		case OP_NOT:
			n = Parser::asnode(t);
			lv = false;
			break;
		default: return postfix();
	}

	l.eat(t);

	return new AST(n, INT, lv ? lval() : unop());
}

// primary [ OP_INC | OP_DEC ]
AST *Parser::postfix()
{
	AST *p = primary();

	TokType t = l.pnxt().type;
	if (t == OP_INC || t == OP_DEC)
	{
		l.eat(t);
		return new AST((t == OP_INC) ? POST_INC : POST_DEC, INT, p);
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
	if (t == INT_CONSTANT)
	{
		l.eat(t);
		return new AST(INT_CONST, INT, std::get<long long>(tok.val));
	}
	else if (t == CHAR_CONSTANT)
	{
		l.eat(t);
		return new AST(INT_CONST, CHAR, std::get<long long>(tok.val));
	}
	else if (t == IDENTIFIER)
	{
		if (l.peek(2).type == LPAREN)
			return call();
		else
			// scope entry and scope id
			return Scope::s(cur_scope)->get(std::get<std::string>(l.eat(IDENTIFIER).val));
	}
	else if (t == LPAREN)
	{
		l.eat(LPAREN);
		AST *out = expr();
		l.eat(RPAREN);

		return out;
	}

	err_tok(std::string("Invalid expression ") + l.getname(t), tok);
}

// IDENTIFIER '(' [ expr { ',' expr } ] ')'
// lhs = var, rhs = params
AST *Parser::call()
{
	AST *out = new AST(CALL);
	out->rhs = new AST(LIST);

	// get symbol from scope
	Token id = l.eat(IDENTIFIER);

	out->lhs = Scope::s(cur_scope)->get(std::get<std::string>(id.val));

	if (out->lhs->get_sym().vtype != V_FUNC)
		err_tok("Attempting to call variable", id);

	l.eat(LPAREN);

	int param_count = 0;
	AST *list_bottom = out->rhs;

	Token tok = l.pnxt();
	TokType t = tok.type;
	while (t != RPAREN)
	{
		list_bottom = AST::append(list_bottom, expr(), LIST);
		++param_count;

		if (l.pnxt().type != RPAREN)
			l.eat(COMMA);
		
		tok = l.pnxt();
		t = tok.type;
	}

	if (!t)
		err_tok("Unclosed parentheses in function call", tok);
	
	if (param_count != out->lhs->get_sym().val)
		err_tok("Too many arguments to function call", tok);

	l.eat(RPAREN);

	return out;
}

// -------- main program blk -------- //

// statementlist = { func | decl }
AST *Parser::parse()
{
	AST *out = new AST(LIST);
	AST *bottom = out;

	TokType t = l.peek(3).type;

	while (l.pnxt().type)
	{
		if (t == LPAREN)
			bottom = AST::append(bottom, func(), LIST);
		else
			bottom = AST::append(bottom, decl(), LIST);

		t = l.peek(3).type;
	}
	
	return out;
}

// omitted: and, inc, dec
// note that mul = ptr and sub = neg
NodeType Parser::asnode(TokType t)
{
	if (t >= OP_SHR_SET && t <= OP_OR_SET)
		return static_cast<NodeType>(SET_SHR + (t - OP_SHR_SET));
	else if (t >= OP_LE && t <= OP_GT)
		return static_cast<NodeType>(N_LE + (t - OP_LE));
	else if (t >= OP_ADD && t <= OP_DIV)
		return static_cast<NodeType>(ADD + (t - OP_ADD));
	else
	{
		switch (t) {
			case OP_SHL: 	return SHL;
			case OP_SHR: 	return SHR;
			case OP_LOGAND: return LOGAND;
			case OP_LOGOR: 	return LOGOR;
			case OP_LOGNOT: return LOGNOT;
			case OP_SET: 	return SET;
			case OP_MOD: 	return MOD;
			case OP_OR: 	return OR;
			case OP_XOR: 	return XOR;
			case OP_NOT: 	return NOT;
			default: return NONE;
		}
	}
}

PrimType Parser::asptype(TokType t)
{
	switch (t) {
		case KEY_INT:
		case INT_CONSTANT:	return INT;
		case KEY_CHAR:
		case CHAR_CONSTANT:	return CHAR;
		case KEY_LONG: 		return LONG;
		default: return INT;
	}
}
