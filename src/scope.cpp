#include <scope.hpp>

std::pair<int, int> Scope::get(const std::string &name)
{
    for (int i = 0; i < syms.size(); ++i)
    {
        if (syms[i].name == name)
            return std::make_pair(i, id);
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
    for (int i = 0; i < syms.size(); ++i)
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
    out->id = scope_count++;
    scopes.push_back(out);

    return out->id;;
}
