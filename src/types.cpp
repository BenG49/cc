#include <types.hpp>

#include <parser.hpp>
#include <err.hpp>

bool compat_types(AST *parent, bool assigning)
{
	PrimType l = parent->lhs->ptype;
	PrimType r = parent->rhs->ptype;
	
	if (l == r)
		return true;

	if (p_sizeof(l) < p_sizeof(r))
	{
		// value will be truncated
		if (assigning)
			warning("value will be truncated. where? idk you wrote it");
		// lvalue will be widened
		else
			parent->lhs = new AST(WIDEN, parent->rhs->ptype, parent->lhs);
	}
	// rvalue will be widened
	else
		parent->rhs = new AST(WIDEN, parent->lhs->ptype, parent->rhs);

	return true;
}

bool compat_types(PrimType out, AST **in)
{
	PrimType i = (*in)->ptype;

	if (out == VOID || i == VOID)
		err("Expression attempted to use void type");
	
	if (out == i)
		return true;
	
	if (p_sizeof(out) < p_sizeof(i))
		// value will be truncated
		warning("value will be truncated. where? idk you wrote it");
	// rvalue will be widened
	else
		*in = new AST(WIDEN, out, *in);

	return true;
}

Size p_sizeof(PrimType t)
{
	switch (t) {
		case INT:   return Long;
		case CHAR:  return Byte;
		case LONG: 	return Quad;
		default:	return Quad;
	}
}
