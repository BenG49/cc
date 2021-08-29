#include <parser.hpp>

// TODO: add some sort of actual memory management
Node *Parser::new_node(NodeType type)
{
    Node *out = new Node;
    out->type = type;
    return out;
}

Node *Parser::expr()
{
    Node *out = new_node(TOKEN);

    if (l.peek_next().type != INT_CONSTANT)
        l.lex_err("Invalid token");
    
    out->tok.t = l.peek_next();
    l.eat(INT_CONSTANT);

    return out;
}

Node *Parser::statement()
{
    Node *out;

    switch (l.peek_next().type) {
        case KEY_RETURN: out = returnstatement(); break;
        default:
            l.lex_err(std::string("Invalid token ") + l.getname(l.peek_next().type));
    }

    l.eat(static_cast<TokType>(';'));

    return out;
}

Node *Parser::returnstatement()
{
    Node *out = new_node(DEFAULT);

    l.eat(KEY_RETURN);

    out->node.lhs = expr();

    return out;
}

// block = '{' statementlist '}' | statement
/*Node *Parser::block()
{
    if (l.peek_next().type == '{')
    {
        Node *out = new_node(LIST);

        while (l.peek_next().type && l.peek_next().type != '}')
                out->listnode.vec.push_back(statement());
        
        l.eat(static_cast<TokType>('}'));

        return out;
    }
    else
        return statement();
}

// ifstatement = IF '(' expr ')' statement
//             | IF '(' expr ')' '{' statementblock '}'
Node *Parser::ifstatement()
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
// function = type IDENTIFIER '(' params ' )' '{' statementlist '}'
Node *Parser::function()
{
    Node *out = new_node(FUNC);

    Token next = l.peek_next();

    Symbol *s = new Symbol;
    s->name = next.s;
    s->type = type();
    out->func.name = s;


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
Node *Parser::parse()
{
    Node *out = new_node(LIST);

    l.eat(KEY_INT);
    std::cout << "aeoigneiangieungieun\n";
    l.eat(IDENTIFIER);
    l.eat(static_cast<TokType>('('));
    l.eat(static_cast<TokType>(')'));
    l.eat(static_cast<TokType>('{'));

    std::cout << l.peek_next().type << '\n';

    while (l.peek_next().type && l.peek_next().type != '}')
        out->listnode.vec.push_back(statement());

    l.eat(static_cast<TokType>('}'));
    
    return out;
}
