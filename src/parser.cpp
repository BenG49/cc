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

#define BINEXP(func, call_func, type_eval)  \
	Expr *Parser::func()						  \
	{											  \
		Expr *out = call_func();				  \
		TokType t = l.peek_next().type;			  \
												  \
		while (type_eval)						  \
		{										  \
			l.eat(t);							  \
			out = new BinOp(t, out, call_func()); \
			t = l.peek_next().type;				  \
		}										  \
												  \
		return out;								  \
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
