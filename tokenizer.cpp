#include "tokenizer.h"
#include <stdexcept>
#include <cctype>
#include <cmath>
#include <set>
#include <map>

// ── All supported functions ───────────────────────────────────────────────────
static const std::set<std::string> KNOWN_FUNCS = {
    // Trig
    "sin", "cos", "tan",
    "asin", "acos", "atan",
    "sinh", "cosh", "tanh",
    // Exponential & log
    "exp",              // e^x
    "ln",               // log base e
    "log",              // log base 10
    "log2",             // log base 2
    // Roots
    "sqrt", "cbrt",     // square root, cube root
    // Misc
    "abs", "ceil", "floor", "sign"
};

// ── Built-in constants ────────────────────────────────────────────────────────
static const std::map<std::string, double> CONSTANTS = {
    {"pi", M_PI},
    {"e",  M_E}
};

bool   Tokenizer::isFunc(const std::string& s)         { return KNOWN_FUNCS.count(s) > 0; }
bool   Tokenizer::isConstant(const std::string& s)     { return CONSTANTS.count(s) > 0; }
double Tokenizer::constantValue(const std::string& s)  { return CONSTANTS.at(s); }

// ─────────────────────────────────────────────
//  IMPLICIT MULTIPLICATION INSERTION
//
//  Handles cases like:
//    3x       → 3 * x
//    2(x+1)   → 2 * (x+1)
//    (x+1)(x-1) → (x+1) * (x-1)
//    sin(x)cos(x) → sin(x) * cos(x)
//    2pi      → 2 * pi
//    3sin(x)  → 3 * sin(x)
//    xsin(x)  → x * sin(x)
//
//  Rule: insert '*' between token A and token B when:
//    A is NUMBER, VARIABLE, CONSTANT, or RPAREN
//    AND
//    B is VARIABLE, CONSTANT, FUNC, or LPAREN
// ─────────────────────────────────────────────
std::vector<Token> Tokenizer::insertImplicitMultiply(const std::vector<Token>& tokens) {
    std::vector<Token> result;
    for (size_t i = 0; i < tokens.size(); ++i) {
        result.push_back(tokens[i]);

        if (i + 1 < tokens.size()) {
            const Token& cur  = tokens[i];
            const Token& next = tokens[i + 1];

            bool curIsValue  = (cur.type  == TokenType::NUMBER   ||
                                cur.type  == TokenType::VARIABLE ||
                                cur.type  == TokenType::CONSTANT ||
                                cur.type  == TokenType::RPAREN);

            bool nextNeedsIt = (next.type == TokenType::VARIABLE ||
                                next.type == TokenType::CONSTANT ||
                                next.type == TokenType::FUNC     ||
                                next.type == TokenType::LPAREN);

            if (curIsValue && nextNeedsIt) {
                result.push_back(Token::makeImplicitMul());
            }
        }
    }
    return result;
}

// ─────────────────────────────────────────────
//  MAIN TOKENIZE
// ─────────────────────────────────────────────
std::vector<Token> Tokenizer::tokenize(const std::string& expr) {
    std::vector<Token> tokens;
    size_t i = 0, len = expr.size();

    while (i < len) {

        // ── Whitespace ────────────────────────────────────────────────
        if (std::isspace(expr[i])) { ++i; continue; }

        // ── Number ────────────────────────────────────────────────────
        if (std::isdigit(expr[i]) || expr[i] == '.') {
            std::string num;
            bool hasDot = false;
            while (i < len && (std::isdigit(expr[i]) || expr[i] == '.')) {
                if (expr[i] == '.') {
                    if (hasDot) throw std::runtime_error("Malformed number: multiple dots");
                    hasDot = true;
                }
                num += expr[i++];
            }
            tokens.push_back(Token::makeNumber(std::stod(num)));
            continue;
        }

        // ── Named token: function, constant, or variable x ───────────
        if (std::isalpha(expr[i])) {
            std::string name;
            while (i < len && std::isalpha(expr[i])) name += expr[i++];

            if (name == "x") {
                tokens.push_back(Token::makeVariable());
            } else if (isConstant(name)) {
                tokens.push_back(Token::makeConstant(name, constantValue(name)));
            } else if (isFunc(name)) {
                tokens.push_back(Token::makeFunc(name));
            } else {
                throw std::runtime_error("Unknown identifier: '" + name + "'");
            }
            continue;
        }

        // ── Operators ─────────────────────────────────────────────────
        if (expr[i]=='+' || expr[i]=='-' ||
            expr[i]=='*' || expr[i]=='/' || expr[i]=='^') {

            // Unary minus → inject 0 before it
            if (expr[i] == '-') {
                bool isUnary = tokens.empty() ||
                               tokens.back().type == TokenType::OPERATOR ||
                               tokens.back().type == TokenType::LPAREN;
                if (isUnary) tokens.push_back(Token::makeNumber(0.0));
            }
            tokens.push_back(Token::makeOp(expr[i++]));
            continue;
        }

        // ── Parentheses ───────────────────────────────────────────────
        if (expr[i] == '(') { tokens.push_back(Token::makeLParen()); ++i; continue; }
        if (expr[i] == ')') { tokens.push_back(Token::makeRParen()); ++i; continue; }

        throw std::runtime_error(std::string("Unexpected character: '") + expr[i] + "'");
    }

    if (tokens.empty()) throw std::runtime_error("Empty expression");

    // ── Insert implicit multiplication ────────────────────────────────
    return insertImplicitMultiply(tokens);
}
