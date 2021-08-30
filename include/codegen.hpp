#pragma once

#include <fstream>
#include <string>

#include <parser.hpp>

void x86_codegen(const std::string &outfile, Block *ast);
