#include <parser.hpp>

void Block::emit(std::ofstream &out) const
{
    for (Stmt *s : vec)
        s->emit(out);
}

void Func::emit(std::ofstream &out) const
{
    out << ".globl " << name_params[0].name << '\n';
    out << name_params[0].name << ":\n";
    blk->emit(out);
}

// all expressions MUST return in rax
void Ret::emit(std::ofstream &out) const
{
    r->emit(out);
    out << "\tret\n";
}

void UnOp::emit(std::ofstream &out) const
{
    switch (op) {
        case '-':
            operand->emit(out);
            out << "\tneg %rax\n";
            break;

        case '~':
            operand->emit(out);
            out << "\tnot %rax\n";
            break;

        case '!':
            operand->emit(out);
            out << "\ttest %rax, %rax\n"; // test if operand is 0
            out << "\tmov $0, %rax\n"; // cannot use xor because it will reset flags
            out << "\tsetz %al\n"; // if operand was zero, set to one
            break;
        // TODO: add some kind of error?
        default: break;
    }
}

void Const::emit(std::ofstream &out) const
{
    out << "\tmov $" <<std::get<long long>(t.val) << ", %rax\n";
}

//

bool Parser::is_type(TokType t)
{
    return t == KEY_BOOL || t == KEY_CHAR || t == KEY_INT || t == KEY_FLOAT || t == KEY_VOID;
}

// binop | const
Expr *Parser::expr()
{
    Expr *out = nullptr;
    TokType t = l.peek_next().type;

    if (t == INT_CONSTANT)
    {
        out = new Const(l.peek_next());
        l.eat(INT_CONSTANT);
    }
    else
        out = unop();

    return out;
}

// func | ret
Stmt *Parser::statement()
{
    Stmt *out = nullptr;
    bool semi = true;

    switch (l.peek_next().type) {
        case KEY_VOID: case KEY_BOOL: case KEY_CHAR: case KEY_INT: case KEY_FLOAT:
            out = function(); semi = false; break;
        case KEY_RETURN: out = returnstatement(); break;
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
    std::string s = std::get<std::string>(l.eat(IDENTIFIER).val);

    out->name_params.emplace_back(t, s);

    l.eat(static_cast<TokType>('('));

    while (l.peek_next().type != ')')
    {
        t = l.peek_next().type;
        if (!is_type(t) && t != KEY_VOID)
            l.lex_err("Expected function param type");
        
        l.eat(t);

        // get param name
        std::string s = std::get<std::string>(l.eat(IDENTIFIER).val);

        out->name_params.emplace_back(t, s);

        if (l.peek_next().type != ')')
            l.eat(static_cast<TokType>(','));
    }

    l.eat(static_cast<TokType>(')'));

    out->blk = block();

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

// ('!' | '~' | '-') unary | primary
Expr *Parser::unop()
{
    UnOp *out = new UnOp;

    TokType t = l.peek_next().type;
    if (t != '!' && t != '~' && t != '-')
			return primary();
    
    l.eat(t);

    out->op = t;
    out->operand = unary();

    return out;
}

// equality
Expr *Parser::binop()
{
	return equality();
}

// comparison ((OP_EQ|OP_NEQ) equality)*
Expr *Parser::equality()
{
	BinOp *out = comparison();

	TokType t = l.peek_next().type;

	while (t == OP_EQ || t == OP_NEQ)
	{
		l.eat(t);

		BinOp *tmp = new BinOp;

		tmp->rhs = equality();
		tmp->lhs = out;

		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// term ((OP_LE|OP_GE|'<'|'>') comparison)*
Expr *Parser::comparison()
{
	BinOp *out = term();

	TokType t = l.peek_next().type;

	while (t == OP_LE || t == OP_GE | t == '<' || t == '>')
	{
		l.eat(t);

		BinOp *tmp = new BinOp;

		tmp->rhs = comparison();
		tmp->lhs = out;

		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// factor (('+'|'-') term)*
Expr *Parser::term()
{
	BinOp *out = factor();

	TokType t = l.peek_next().type;

	while (t == '+' || t == '-')
	{
		l.eat(t);

		BinOp *tmp = new BinOp;

		tmp->rhs = term();
		tmp->lhs = out;

		out = tmp;

		t = l.peek_next().type;
	}

	return out;
}

// unop (('/'|'*') factor)*
Expr *Parser::factor()
{
	BinOp *out = unop();

	TokType t = l.peek_next().type;

	while (t == '/' || t == '*')
	{
		l.eat(t);

		BinOp *tmp = new BinOp;

		tmp->rhs = factor();
		tmp->lhs = out;

		out = tmp;

		t = l.peek_next().type;
	}

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
		Const *out = new Const;
		out->t = tok;
		return out;
	}
	else if (t == IDENTIFIER)
		return new Symbol(t, std::get<std::string>(tok.val));
	else if (t == '(')
	{
		l.eat(static_cast<TokType>('('));
		Expr *out = expr();
		l.eat(static_cast<TokType>(')'));

		return out;
	}
	else
		lex_err(std::string("Invalid expression ") + l.getname(t));
}


// ifstatement = IF '(' expr ')' statement
//             | IF '(' expr ')' '{' statementblock '}'
/*Node *Parser::ifstatement()
{
    Node *out = new_node(IF);

    l.eat(KEY_IF);
    l.eat(static_cast<TokType>('('));

    out->ifnode.cond = expr();

    l.eat(static_cast<TokType>(')'));

    out->ifnode.bodies.push_back(block());
    
    for (;;)
    {
        // not else
        if (l.peek_next().type != KEY_ELSE)
            break;
        
        l.eat(KEY_ELSE);

        // else if
        if (l.peek_next().type == KEY_IF)
            out->ifnode.bodies.push_back(ifstatement());
        // else
        else
        {
            out->ifnode.bodies.push_back(block());
            
            break;
        }
    }
    
    return out;
}

// TODO: add symtable
Node *Parser::decl()
{

}

// type = KEY_INT | KEY_FLOAT | KEY_CHAR | KEY_BOOL | KEY_VOID
TokType Parser::type()
{
    TokType next = l.peek_next().type;
    if (next == KEY_INT || next == KEY_FLOAT || next == KEY_CHAR || next == KEY_BOOL || next == KEY_VOID)
        return next;

    l.lex_err("Expected type");
}*/

//

// statementlist = statement statementlist | statement EOF
Expr *Parser::parse()
{
    Block *out = new Block;

    while (l.peek_next().type)
        out->vec.push_back(statement());
    
    return out;
}
