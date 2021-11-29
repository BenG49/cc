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
		case CHAR:  return Byte;
		case INT:   return Long;
		case CHAR_PTR:
		case LONG_PTR:
		case INT_PTR:
		case LONG: 	return Quad;
		default:
			err("Attempted to get size of invalid type");
	}

	// supress warning
	return Byte;
}

PrimType pointer_type(PrimType t)
{
	switch (t) {
		case VOID: return VOID_PTR;
		case INT:  return INT_PTR;
		case CHAR: return CHAR_PTR;
		case LONG: return LONG_PTR;
		default:
			err("Invalid pointer_to type");
	}

	// supress warning
	return VOID;
}
