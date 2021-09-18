#include <iostream>

#include <codegen.hpp>
#include <scope.hpp>
#include <types.hpp>
#include <err.hpp>

void prettyprint(const AST *ast, int tabs)
{
	if (!ast)
		return;

	for (int i = 0; i < tabs; ++i)
		printf("  ");

	if (ast->val)
		printf("%s %d ptype: %d\n", NODE_NAMES[ast->type], ast->val, ast->ptype);
	else if (ast->type == VAR)
		printf("%s %s ptype: %d\n", NODE_NAMES[ast->type], ast->get_sym().name.c_str(), ast->ptype);
	else
		printf("%s\n", NODE_NAMES[ast->type]);

	prettyprint(ast->lhs, tabs + 1);
	prettyprint(ast->mid, tabs + 1);
	prettyprint(ast->rhs, tabs + 1);
}

int main(int argc, const char *argv[]) {
	if (argc == 1)
	{
		std::cerr << "Must specify input file!\n";
		return 1;
	}

	Lexer l(argv[1]);

	Parser p(l);
	AST *ast = p.parse();

	// prettyprint(ast, 0);

	init_cg("out.s");
	gen_ast(ast, Ctx(NOREG, NONE, 0, 0, 0));
	gen_globls();
}
