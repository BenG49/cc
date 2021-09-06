#include <iostream>

#include <parser.hpp>

void ptabs(int count)
{
	for (int i = 0; i < count; ++i)
		printf("  ");
}

void prettyprint(const Node *ast, int tabs, const Scope &s)
{
	ptabs(tabs);

	if (!ast)
	{
		puts("0");
		return;
	}

	switch (ast->type) {
		case BLOCK:
			puts("BLOCK:");
			for (Node *n : ((Compound*)ast)->vec)
				prettyprint(n, tabs + 1, s);
			break;

		case IF:
			puts("IF");
			prettyprint(((If*)ast)->cond, tabs + 1, s);

			ptabs(tabs);
			puts("THEN");
			prettyprint(((If*)ast)->if_blk, tabs + 1, s);

			if (((If*)ast)->else_blk)
			{
				ptabs(tabs);
				puts("ELSE");
				prettyprint(((If*)ast)->else_blk, tabs + 1, s);
			}

			break;

		case FUNC:
			printf("FUNC TYPE SIZE(%d)", ((Func*)ast)->name.get().size);
			printf(" NAME(%s)", ((Func*)ast)->name.get().name.c_str());
			puts(" PARAMS:");
			
			for (unsigned i = 0; i < ((Func*)ast)->params.size(); ++i)
			{
				Var s = ((Func*)ast)->params.at(i);
				ptabs(tabs + 1);
				printf("%d %s\n", s.type, s.get().name.c_str());
			}

			prettyprint(((Func*)ast)->blk, tabs + 1, s);
			break;

		case RET:
			puts("RET:");
			prettyprint(((Ret*)ast)->r, tabs + 1, s);
			break;
		
		case BINOP:
			printf("OP: ");
			puts(Lexer::getname(((BinOp*)ast)->op));
			prettyprint(((BinOp*)ast)->lhs, tabs + 1, s);
			prettyprint(((BinOp*)ast)->rhs, tabs + 1, s);
			break;
		
		case UNOP:
			printf("OP: ");
			puts(Lexer::getname(((UnOp*)ast)->op));
			prettyprint(((UnOp*)ast)->operand, tabs + 1, s);
			break;
		
		case CONST:
			printf("CONST: ");
			switch (((Const*)ast)->t.type) {
				case INT_CONSTANT: printf("%lld", std::get<long long>(((Const*)ast)->t.val)); break;
				case FP_CONSTANT:  printf("%f", std::get<double>(((Const*)ast)->t.val)); break;
				case STR_CONSTANT: printf("%s", std::get<std::string>(((Const*)ast)->t.val).c_str()); break;
				default: printf("cringe");
			}

			putchar('\n');

			break;
		
		case DECL:
			if (((Decl*)ast)->v->type == GLOBL)
			{
				printf("DECL TYPE SIZE(%d) NAME(%s)\n",
					((Globl*)((Decl*)ast)->v)->get().size,
					((Globl*)((Decl*)ast)->v)->get().name.c_str());
			}
			else
				printf("DECL TYPE SIZE(%d) NAME(%s)\n",
					((Var*)((Decl*)ast)->v)->get().size,
					((Var*)((Decl*)ast)->v)->get().name.c_str());
			
			break;
		
		case VAR:
			printf("VAR NAME(%s)\n", ((Var*)ast)->get().name.c_str());
			break;
		
		case COND:
			puts("IF");
			prettyprint(((Cond*)ast)->cond, tabs + 1, s);

			ptabs(tabs);
			puts("THEN");
			prettyprint(((Cond*)ast)->t, tabs + 1, s);

			ptabs(tabs);
			puts("ELSE");
			prettyprint(((Cond*)ast)->f, tabs + 1, s);

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
	Compound *b = p.parse();

	// prettyprint(b, 0, s);

	Gen g("out.s");
	g.x86_codegen(b);
}
