#include <parser.hpp>

#include <symtab.hpp>

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

// -------- grammars -------- //

// binop | assign
Expr *Parser::expr()
{
	if (is_assign(l.peek(2).type))
	 	return assign();
	else
		return op_or();
}

// ret | expr | decl
Node *Parser::statement()
{
	Node *out = nullptr;

	switch (l.peek_next().type) {
		case KEY_RETURN: out = returnstatement(); break;
		case KEY_VOID: case KEY_BOOL: case KEY_CHAR: case KEY_INT: case KEY_FLOAT:
			out = decl(); break;
		default:
			out = expr(); break;

		// case IDENTIFIER: case INT_CONSTANT: case FP_CONSTANT: case STR_CONSTANT: case '!': case '~': case '-': case '(':
			// l.lex_err(std::string("Invalid token ") + l.getname(l.peek_next().type));
	}

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

// type IDENTIFIER '(' {type IDENTIFIER,} ')' '{' blk '}'
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

// IDENTIFIER
Expr *Parser::lval()
{
	return new Var(s.lookup(std::get<std::string>(l.eat(IDENTIFIER).val)));
}

// lval [ assign expr ]
Expr *Parser::assign()
{
	Expr *lv = lval();

	TokType t = l.peek_next().type;
	if (!is_assign(t))
		l.lex_err("Expected assignment operator");
	
	l.eat(t);

	return new Assign(t, lv, expr());
}

// -------- binary operations -------- //

// op_and { '&&' op_and }
Expr *Parser::op_or()
{
	Expr *out = op_and();

	TokType t = l.peek_next().type;

	while (t == OP_AND)
	{
		l.eat(OP_AND);

		out = new BinOp(OP_AND, out, op_and());

		t = l.peek_next().type;
	}

	return out;
}

// bitwise_or { '||' bitwise_or }
Expr *Parser::op_and()
{
	Expr *out = bitwise_or();

	TokType t = l.peek_next().type;

	while (t == OP_OR)
	{
		l.eat(OP_OR);

		out = new BinOp(OP_OR, out, bitwise_or());

		t = l.peek_next().type;
	}

	return out;
}

// bitwise_xor { '|' bitwise_xor }
Expr *Parser::bitwise_or()
{
	Expr *out = bitwise_xor();

	TokType t = l.peek_next().type;

	while (t == '|')
	{
		l.eat(static_cast<TokType>('|'));

		out = new BinOp(static_cast<TokType>('|'), out, bitwise_xor());

		t = l.peek_next().type;
	}

	return out;
}

// bitwise_and { '^' bitwise_and }
Expr *Parser::bitwise_xor()
{
	Expr *out = bitwise_and();

	TokType t = l.peek_next().type;

	while (t == '^')
	{
		l.eat(static_cast<TokType>('^'));

		out = new BinOp(static_cast<TokType>('^'), out, bitwise_and());

		t = l.peek_next().type;
	}

	return out;
}

// equality { '&' equality }
Expr *Parser::bitwise_and()
{
	Expr *out = equality();

	TokType t = l.peek_next().type;

	while (t == '&')
	{
		l.eat(static_cast<TokType>('&'));

		out = new BinOp(static_cast<TokType>('&'), out, equality());

		t = l.peek_next().type;
	}

	return out;
}

// comparison { ( OP_EQ | OP_NE ) comparison }
Expr *Parser::equality()
{
	Expr *out = comparison();

	TokType t = l.peek_next().type;

	while (t == OP_EQ || t == OP_NE)
	{
		l.eat(t);

		out = new BinOp(t, out, comparison());

		t = l.peek_next().type;
	}

	return out;
}

// shift { ( OP_LE | OP_GE | '<' | '>' ) shift }
Expr *Parser::comparison()
{
	Expr *out = shift();

	TokType t = l.peek_next().type;

	while (t == OP_LE || t == OP_GE || t == '<' || t == '>')
	{
		l.eat(t);

		out = new BinOp(t, out, shift());

		t = l.peek_next().type;
	}

	return out;
}

// term { ( OP_SHR | OP_SHL ) term }
Expr *Parser::shift()
{
	Expr *out = term();

	TokType t = l.peek_next().type;

	while (t == OP_SHL || t == OP_SHR)
	{
		l.eat(t);

		out = new BinOp(t, out, term());

		t = l.peek_next().type;
	}

	return out;
}

// factor { ( '+' | '-' ) factor }
Expr *Parser::term()
{
	Expr *out = factor();

	TokType t = l.peek_next().type;

	while (t == '+' || t == '-')
	{
		l.eat(t);

		out = new BinOp(t, out, factor());

		t = l.peek_next().type;
	}

	return out;
}

// unop { ( '/' | '*' | '%' ) unop }
Expr *Parser::factor()
{
	Expr *out = unop();

	TokType t = l.peek_next().type;

	while (t == '/' || t == '*' || t == '%')
	{
		l.eat(t);

		out = new BinOp(t, out, unop());

		t = l.peek_next().type;
	}

	return out;
}

// postfix | ( OP_INC | OP_DEC ) lval | ( '!' | '~' | '-' ) unop
Expr *Parser::unop()
{
	TokType t = l.peek_next().type;
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

	TokType t = l.peek_next().type;
	std::cout << "postfix: " << Lexer::getname(t) << '\n';
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

// -------- main program blk -------- //

// statementlist = function statementlist | function EOF
Block *Parser::parse()
{
	Block *out = new Block;

	while (l.peek_next().type)
		out->vec.push_back(function());
	
	return out;
}
