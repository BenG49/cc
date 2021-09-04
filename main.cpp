#include <iostream>

#include <parser.hpp>

void ptabs(int count)
{
	for (int i = 0; i < count; ++i)
		printf("  ");
}

void prettyprint(const Node *ast, int tabs, const SymTab &s)
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
			printf("FUNC TYPE(%s)", Lexer::getname(s.vec[((Func*)ast)->name.entry].type));
			printf(" NAME(%s)", s.vec[((Func*)ast)->name.entry].name.c_str());
			puts(" PARAMS:");
			
			for (unsigned i = 0; i < ((Func*)ast)->params.size(); ++i)
			{
				Symbol s = ((Func*)ast)->params.at(i);
				ptabs(tabs + 1);
				printf("%d %s\n", s.type, s.name.c_str());
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
			printf("DECL TYPE(%s) NAME(%s)\n",
				Lexer::getname(s.vec[((Decl*)ast)->v.entry].type),
				s.vec[((Decl*)ast)->v.entry].name.c_str());
			
			break;
		
		case VAR:
			printf("VAR NAME(%s)\n", s.vec[((Var*)ast)->entry].name.c_str());
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
		std::cout << "Must specify input file!\n";
		return 1;
	}

	Lexer l(argv[1]);
	Parser p(l);
	Compound *b = p.parse();

	// prettyprint(b, 0, s);

	Gen g("out.s");
	g.x86_codegen(b);
}
