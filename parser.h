#pragma once
#include "tokenizer.h"
#include <vector>

// ─────────────────────────────────────────────
//  PARSER
//  Converts infix token list → postfix (RPN)
//  using the Shunting Yard Algorithm
// ─────────────────────────────────────────────
class Parser {
public:
    // Returns postfix token list; throws std::runtime_error on bad expression
    std::vector<Token> toPostfix(const std::vector<Token>& infix);

private:
    int  precedence(char op);
    bool isRightAssoc(char op);   // only '^' is right-associative
};
