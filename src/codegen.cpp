#include <codegen.hpp>

void x86_codegen(const std::string &outfile, Block *ast)
{
    std::ofstream out(outfile);

    ast->emit(out);

    out.close();
}
