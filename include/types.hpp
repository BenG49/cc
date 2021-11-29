#pragma once

#include <codegen.hpp>
#include <defs.hpp>

struct AST;

/**
 * For me to understand:
 * Method is given two ast nodes which are interacting in some
 * way, and their primtype is replaced with NO_WIDEN if no widening
 * is neccessary. If widening is neccessary, the node is replaced with
 * a WIDEN node with its type set to the type to widen to
 */
bool compat_types(AST *parent, bool assigning);
bool compat_types(PrimType out, AST **in);
Size p_sizeof(PrimType t);

PrimType pointer_type(PrimType t);

