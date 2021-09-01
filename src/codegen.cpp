#include <codegen.hpp>
#include <parser.hpp>

const char *REG_64[REG_COUNT] = { "rax", "rbx", "rcx", "rdx", "rsp", "rbp", "rdi", "rsi", "rip" };
const char *REG_32[REG_COUNT] = { "eax", "ebx", "ecx", "edx", "esp", "ebp", "edi", "esi", "eip" };
const char *REG_16[REG_COUNT] = { "ax", "bx", "cx", "dx", "sp", "bp", "di", "si", "ip" };
const char *REG_H[GP_REGS_END] = { "ah", "bh", "ch", "dh" };
const char *REG_L[REG_COUNT - 1] = { "al", "bl", "cl", "dl", "spl", "bpl", "dil", "sil" };

void Gen::x86_codegen(Block *ast)
{
	ast->emit(*this);
}

void Gen::emit(const char *str, bool nl)
{
	out << '\t' << str;
	if (nl) out << '\n';
}

void Gen::emit_append(const char *str, bool nl)
{
	out << str;
	if (nl) out << '\n';
}

void Gen::emit_int(long long i) { out << i; }
void Gen::emit_fp(double f) { out << f; }

void Gen::comment(const char *str)
{
	out << "# " << str << '\n';
};

void Gen::func_epilogue()
{
	out << "\n\tmov %rbp, %rsp\n"
		<< "\tpop %rbp\n"
		<< "\tret\n";
}

const char *Gen::get_label()
{
	char *buf = new char[10];
	// yes
	buf[0] = 'u';
	buf[1] = 'w';
	buf[2] = 'u';

	std::string tmp = std::to_string(label++);

	for (unsigned i = 0; i < tmp.size(); ++i)
		buf[i + 3] = tmp[i];

	buf[3 + tmp.size()] = '\0';

	return buf;
}
