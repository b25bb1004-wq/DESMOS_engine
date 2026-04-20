#pragma once
#include <string>
#include <vector>

enum class TokenType {
    NUMBER,
    VARIABLE,
    CONSTANT,   // pi, e
    OPERATOR,   // + - * / ^
    FUNC,       // sin, cos, exp, ln ...
    LPAREN,
    RPAREN
};

struct Token {
    TokenType   type;
    double      numVal;
    char        op;
    std::string name;

    static Token makeNumber(double v)                          { Token t; t.type=TokenType::NUMBER;   t.numVal=v; t.op=0;   return t; }
    static Token makeVariable()                                { Token t; t.type=TokenType::VARIABLE; t.numVal=0; t.op=0;   t.name="x"; return t; }
    static Token makeConstant(const std::string& n, double v) { Token t; t.type=TokenType::CONSTANT; t.numVal=v; t.op=0;   t.name=n; return t; }
    static Token makeOp(char c)                                { Token t; t.type=TokenType::OPERATOR; t.numVal=0; t.op=c;   return t; }
    static Token makeFunc(const std::string& n)                { Token t; t.type=TokenType::FUNC;     t.numVal=0; t.op=0;   t.name=n; return t; }
    static Token makeLParen()                                  { Token t; t.type=TokenType::LPAREN;   t.numVal=0; t.op='('; return t; }
    static Token makeRParen()                                  { Token t; t.type=TokenType::RPAREN;   t.numVal=0; t.op=')'; return t; }
    static Token makeImplicitMul()                             { return makeOp('*'); }
};

class Tokenizer {
public:
    std::vector<Token> tokenize(const std::string& expr);
private:
    bool   isFunc(const std::string& s);
    bool   isConstant(const std::string& s);
    double constantValue(const std::string& s);
    // Inserts implicit '*' where needed (e.g. 3x, 2(x+1), (x+1)(x-1))
    std::vector<Token> insertImplicitMultiply(const std::vector<Token>& tokens);
};
