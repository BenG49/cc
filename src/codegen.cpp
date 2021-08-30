#include <codegen.hpp>

void x86_codegen(const std::string &outfile, Block *ast)
{
    std::ofstream out(outfile);

    // main function
    out << ".globl main\nmain:\n";
    long long i = std::get<long long>(((Leaf *)((Ret *)ast->vec.at(0))->r)->t.val);
    out << "\tmov $" << i << ", %rax\n";
    out << "\tret\n";

    out.close();
}
