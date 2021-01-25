#pragma once

#include "lexer.h"
#include "statement.h"
#include "comparators.h"

#include <memory>
#include <stdexcept>

namespace Ast {
  class Statement;
}

namespace Parse {
  class Lexer;
}

class TestRunner;

struct ParseError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

std::unique_ptr<Ast::Statement> ParseProgram(Parse::Lexer& lexer);

void TestParseProgram(TestRunner& tr);
