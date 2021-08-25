#include <parser.hpp>

Parser::Parser(Lexer &l)
    : l(l) {}

void Parser::parse_err(const std::string &msg)
{
    // TODO: maybe replace this? idk
    l.lex_err(msg);
}

AST Parser::primary_expr()
{
    Token next = l.peek_next();

    if (next.type == IDENTIFIER || next.type == CONSTANT || next.type == STRING_LITERAL)
        return ASTLeaf(next);
    else if (next.type == '(')
    {
        l.eat(static_cast<TokType>('('));
        Token out = expr();
        l.eat(static_cast<TokType>(')'));
        return out;
    }
    else
        parse_err("Unexpeced identifier");
}

AST postfix_expr()
{
    
}
