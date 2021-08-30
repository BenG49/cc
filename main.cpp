#include <iostream>

#include <codegen.hpp>

int main(int argc, const char *argv[]) {
    if (argc == 1)
    {
        std::cout << "Must specify input file!\n";
        return 1;
    }

    Lexer l(argv[1]);
    Parser p(l);
    x86_codegen("out.S", p.parse());

    /*Token t = l.peek_next();
    while (t.type)
    {
        t = l.peek_next();

        if (t.type < 256)
            std::cout << "TOK " << (char)t.type;
        else if (t.type < IDENTIFIER)
        {
            // std::cout << t.type << '\n';
            std::cout << "TOK " << l.getname(t.type);
        }
        else
            std::cout << "TOK " << t.type;

        if (t.type == INT_CONSTANT)
            std::cout << " val " << t.i;
        else if (t.type == FP_CONSTANT)
            std::cout << " val " << t.f;
        else if (t.type == STR_CONSTANT || t.type == IDENTIFIER)
            std::cout << " val " << t.s;

        std::cout << '\n';

        l.eat(t.type);
    }*/
}
