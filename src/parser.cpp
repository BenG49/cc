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

// const | unop
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
Ret *Parser::returnstatement()
{
    l.eat(KEY_RETURN);

    Ret *out = new Ret;

    out->r = expr();

    return out;
}

// type IDENTIFIER '(' (type IDENTIFIER,)* ')' '{' blk '}'
Func *Parser::function()
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
Block *Parser::block()
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

// ('!' | '~' | '-') expr
UnOp *Parser::unop()
{
    UnOp *out = new UnOp;

    TokType t = l.peek_next().type;
    if (t != '!' && t != '~' && t != '-')
        l.lex_err("Expected unary operator");
    
    l.eat(t);

    out->op = t;
    out->operand = expr();

    return out;
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
Block *Parser::parse()
{
    Block *out = new Block;

    while (l.peek_next().type)
        out->vec.push_back(statement());
    
    return out;
}
