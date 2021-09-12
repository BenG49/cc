#include <scope.hpp>
#include <parser.hpp>

int Scope::scope_count = 0;

AST *Scope::get(const std::string &name)
{
    for (unsigned i = 0; i < syms.size(); ++i)
    {
        if (syms[i].name == name)
            return new AST(i, id);
    }

    if (parent_id >= 0)
        return Scope::scopes[parent_id]->get(name);
    else
    {
        std::cerr << "Could not find variable " << name << '\n';
        exit(1);
    }
}

bool Scope::in_scope(const std::string &name)
{
    for (unsigned i = 0; i < syms.size(); ++i)
    {
        if (syms[i].name == name)
            return true;
    }

    return false;
}

//

std::vector<Scope*> Scope::scopes;

int Scope::new_scope(int cur)
{
    Scope *out = new Scope;
    out->parent_id = cur;
    out->id = Scope::scope_count++;
    scopes.push_back(out);

    return out->id;;
}
