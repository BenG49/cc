#include <iostream>

#include <codegen.hpp>
#include <scope.hpp>

void ptabs(int count)
{
	for (int i = 0; i < count; ++i)
		printf("  ");
}

void prettyprint(const AST *ast, int tabs)
{
	if (!ast)
	{
		puts("");
		return;
	}

	ptabs(tabs);

	switch (ast->type) {
		case LIST: {
			puts("LIST:");

			ASTIter i(ast);
			while (i.has_next())
				prettyprint(i.next(), tabs + 1);

			break;
		}

		case IF:
			puts("IF");
			prettyprint(ast->lhs, tabs + 1);

			ptabs(tabs);
			puts("THEN");
			prettyprint(ast->mid, tabs + 1);

			if (ast->rhs)
			{
				ptabs(tabs);
				puts("ELSE");
				prettyprint(ast->rhs, tabs + 1);
			}

			break;

		case FUNC: {
			Sym &s = Scope::s(ast->lhs->scope_id)->syms[ast->lhs->val];
			printf("FUNC %s %s", Lexer::getname(s.type), s.name.c_str());
			puts(" PARAMS:");
			
			ASTIter i(ast->mid);
			while (i.has_next())
			{
				ptabs(tabs + 1);
				AST *a = i.next();
				Sym &s = Scope::s(a->scope_id)->syms[a->val];
				printf("%s %s\n", Lexer::getname(s.type), s.name.c_str());
			}

			prettyprint(ast->rhs, tabs + 1);
			break;
		}

		case RET:
			puts("RET:");
			prettyprint(ast->lhs, tabs + 1);
			break;
		
		case BINOP:
			printf("OP: ");
			puts(Lexer::getname(ast->op));
			prettyprint(ast->lhs, tabs + 1);
			prettyprint(ast->rhs, tabs + 1);
			break;
		
		case UNOP:
			printf("OP: ");
			puts(Lexer::getname(ast->op));
			prettyprint(ast->lhs, tabs + 1);
			break;
		
		case CONST:
			printf("CONST: ");
			switch (ast->op) {
				case INT_CONSTANT: printf("%d", ast->val); break;
				// case FP_CONSTANT:  printf("%f", std::get<double>(((Const*)ast)->t.val)); break;
				// case STR_CONSTANT: printf("%s", std::get<std::string>(((Const*)ast)->t.val).c_str()); break;
				default: printf("cringe");
			}

			putchar('\n');

			break;
		
		case DECL: {
			Sym &s = Scope::s(ast->lhs->scope_id)->syms[ast->lhs->val];
			printf("DECL %s %s\n", Lexer::getname(s.type), s.name.c_str());
			if (ast->rhs)
				prettyprint(ast->rhs, tabs + 1);
			break;
		}
		
		case VAR:
			printf("VAR %s\n", Scope::s(ast->scope_id)->syms[ast->val].name.c_str());
			break;
		
		case COND:
			puts("IF");
			prettyprint(ast->lhs, tabs + 1);

			ptabs(tabs);
			puts("THEN");
			prettyprint(ast->mid, tabs + 1);

			if (ast->rhs)
			{
				ptabs(tabs);
				puts("ELSE");
				prettyprint(ast->rhs, tabs + 1);
			}

			break;
		
		case ASSIGN:
			puts(Lexer::getname(ast->op));
			
			prettyprint(ast->lhs, tabs + 1);
			prettyprint(ast->rhs, tabs + 1);

			break;
		
		case FOR: case FOR_DECL:
			puts("FOR");

			prettyprint(ast->lhs, tabs + 1);
			prettyprint(ast->mid, tabs + 1);
			prettyprint(ast->rhs->lhs, tabs + 1);
			prettyprint(ast->rhs->rhs, tabs + 1);
			break;
		
		case POSTFIX:
			printf("POST %s\n", Lexer::getname(ast->op));
			prettyprint(ast->lhs, tabs + 1);

			break;
		
		case CALL:
			printf("CALL FUNC %s\n", ast->lhs->get_sym().name.c_str());
			prettyprint(ast->rhs, tabs + 1);
			break;
		
		default: printf("megacringe %d\n", ast->type); break;
	};
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
