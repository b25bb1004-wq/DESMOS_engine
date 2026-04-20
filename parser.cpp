#include "parser.h"
#include <stack>
#include <stdexcept>

// ─────────────────────────────────────────────
//  Operator precedence table
//  Higher number = higher precedence
// ─────────────────────────────────────────────
int Parser::precedence(char op) {
    switch (op) {
        case '+': case '-': return 1;
        case '*': case '/': return 2;
        case '^':           return 3;
        default:            return 0;
    }
}

// Only ^ is right-associative (e.g. 2^3^2 = 2^(3^2))
bool Parser::isRightAssoc(char op) {
    return op == '^';
}

// ─────────────────────────────────────────────
//  Shunting Yard Algorithm
//  Ref: https://en.wikipedia.org/wiki/Shunting-yard_algorithm
//
//  Rules:
//  • Numbers / variables   → output queue directly
//  • Functions             → operator stack
//  • Operators             → pop stack to output respecting precedence
//                            and associativity, then push
//  • '('                   → push onto operator stack
//  • ')'                   → pop stack to output until matching '('
// ─────────────────────────────────────────────
std::vector<Token> Parser::toPostfix(const std::vector<Token>& infix) {
    std::vector<Token> output;       // RPN result
    std::stack<Token>  opStack;      // operator / function stack

    for (const Token& tok : infix) {

        switch (tok.type) {

            // ── Operands go straight to output ───────────────────────
            case TokenType::NUMBER:
            case TokenType::VARIABLE:
            case TokenType::CONSTANT:
                output.push_back(tok);
                break;

            // ── Functions pushed onto op stack ────────────────────────
            case TokenType::FUNC:
                opStack.push(tok);
                break;

            // ── Operators ─────────────────────────────────────────────
            case TokenType::OPERATOR: {
                while (!opStack.empty()) {
                    const Token& top = opStack.top();

                    // Pop if top is a function
                    bool topIsFunc = (top.type == TokenType::FUNC);

                    // Pop if top is an operator with higher precedence,
                    // OR same precedence and current op is left-associative
                    bool topIsOp = (top.type == TokenType::OPERATOR);
                    bool shouldPop = topIsFunc ||
                        (topIsOp && (
                            precedence(top.op) > precedence(tok.op) ||
                            (precedence(top.op) == precedence(tok.op) && !isRightAssoc(tok.op))
                        ));

                    if (!shouldPop || top.type == TokenType::LPAREN) break;

                    output.push_back(top);
                    opStack.pop();
                }
                opStack.push(tok);
                break;
            }

            // ── Left parenthesis → push ───────────────────────────────
            case TokenType::LPAREN:
                opStack.push(tok);
                break;

            // ── Right parenthesis → pop until matching '(' ────────────
            case TokenType::RPAREN: {
                bool foundLeft = false;
                while (!opStack.empty()) {
                    Token top = opStack.top();
                    opStack.pop();
                    if (top.type == TokenType::LPAREN) { foundLeft = true; break; }
                    output.push_back(top);
                }
                if (!foundLeft)
                    throw std::runtime_error("Mismatched parentheses: extra ')'");

                // If top of stack is now a function, pop it too
                if (!opStack.empty() && opStack.top().type == TokenType::FUNC) {
                    output.push_back(opStack.top());
                    opStack.pop();
                }
                break;
            }
        }
    }

    // ── Drain remaining operators ─────────────────────────────────────
    while (!opStack.empty()) {
        Token top = opStack.top();
        opStack.pop();
        if (top.type == TokenType::LPAREN || top.type == TokenType::RPAREN)
            throw std::runtime_error("Mismatched parentheses: extra '('");
        output.push_back(top);
    }

    return output;
}
