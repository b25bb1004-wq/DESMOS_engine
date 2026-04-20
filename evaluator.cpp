#include "evaluator.h"
#include <stack>
#include <cmath>
#include <stdexcept>
#include <limits>

// ─────────────────────────────────────────────
//  BINARY OPERATOR APPLICATION
// ─────────────────────────────────────────────
static double applyOp(char op, double a, double b) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (std::abs(b) < 1e-12)
                return std::numeric_limits<double>::quiet_NaN();
            return a / b;
        case '^': return std::pow(a, b);
        default:
            throw std::runtime_error(std::string("Unknown operator: ") + op);
    }
}

// ─────────────────────────────────────────────
//  FUNCTION APPLICATION
//  Supports: trig, inverse trig, hyperbolic,
//            exp, ln, log, log2,
//            sqrt, cbrt, abs, ceil, floor, sign
// ─────────────────────────────────────────────
static double applyFunc(const std::string& name, double v) {
    // ── Trigonometric ─────────────────────────
    if (name == "sin")   return std::sin(v);
    if (name == "cos")   return std::cos(v);
    if (name == "tan")   return std::tan(v);
    // ── Inverse trig ──────────────────────────
    if (name == "asin")  {
        if (v < -1.0 || v > 1.0) return std::numeric_limits<double>::quiet_NaN();
        return std::asin(v);
    }
    if (name == "acos")  {
        if (v < -1.0 || v > 1.0) return std::numeric_limits<double>::quiet_NaN();
        return std::acos(v);
    }
    if (name == "atan")  return std::atan(v);
    // ── Hyperbolic ────────────────────────────
    if (name == "sinh")  return std::sinh(v);
    if (name == "cosh")  return std::cosh(v);
    if (name == "tanh")  return std::tanh(v);
    // ── Exponential ───────────────────────────
    if (name == "exp")   return std::exp(v);       // e^v
    // ── Logarithms ───────────────────────────
    if (name == "ln")    {
        if (v <= 0) return std::numeric_limits<double>::quiet_NaN();
        return std::log(v);
    }
    if (name == "log")   {
        if (v <= 0) return std::numeric_limits<double>::quiet_NaN();
        return std::log10(v);
    }
    if (name == "log2")  {
        if (v <= 0) return std::numeric_limits<double>::quiet_NaN();
        return std::log2(v);
    }
    // ── Roots ─────────────────────────────────
    if (name == "sqrt")  {
        if (v < 0) return std::numeric_limits<double>::quiet_NaN();
        return std::sqrt(v);
    }
    if (name == "cbrt")  return std::cbrt(v);       // cube root (works for negatives)
    // ── Misc ──────────────────────────────────
    if (name == "abs")   return std::abs(v);
    if (name == "ceil")  return std::ceil(v);
    if (name == "floor") return std::floor(v);
    if (name == "sign")  return (v > 0) - (v < 0); // -1, 0, or 1

    throw std::runtime_error("Unknown function: " + name);
}

// ─────────────────────────────────────────────
//  POSTFIX (RPN) EVALUATION
// ─────────────────────────────────────────────
double Evaluator::evaluate(const std::vector<Token>& postfix, double xVal) {
    std::stack<double> s;

    for (const Token& tok : postfix) {
        switch (tok.type) {

            case TokenType::NUMBER:
                s.push(tok.numVal);
                break;

            case TokenType::VARIABLE:
                s.push(xVal);
                break;

            // Constants (pi, e) stored as their numeric value
            case TokenType::CONSTANT:
                s.push(tok.numVal);
                break;

            case TokenType::OPERATOR: {
                if (s.size() < 2)
                    throw std::runtime_error("Malformed expression: not enough operands");
                double b = s.top(); s.pop();
                double a = s.top(); s.pop();
                s.push(applyOp(tok.op, a, b));
                break;
            }

            case TokenType::FUNC: {
                if (s.empty())
                    throw std::runtime_error("Missing argument for function: " + tok.name);
                double arg = s.top(); s.pop();
                s.push(applyFunc(tok.name, arg));
                break;
            }

            default:
                throw std::runtime_error("Unexpected token in postfix expression");
        }
    }

    if (s.size() != 1)
        throw std::runtime_error("Malformed expression: too many operands");

    return s.top();
}

// ─────────────────────────────────────────────
//  POINT GENERATION  — loop x over [min, max]
// ─────────────────────────────────────────────
std::vector<Point> Evaluator::generatePoints(
    const std::vector<Token>& postfix,
    double xMin, double xMax, double step)
{
    if (step <= 0)      throw std::runtime_error("Step size must be positive");
    if (xMin >= xMax)   throw std::runtime_error("xMin must be less than xMax");

    std::vector<Point> pts;
    pts.reserve(static_cast<size_t>((xMax - xMin) / step) + 2);

    for (double x = xMin; x <= xMax + 1e-9; x += step) {
        double y = evaluate(postfix, x);
        // Skip undefined / infinite values (div/0, sqrt(-n), log(0), etc.)
        if (std::isnan(y) || std::isinf(y)) continue;
        pts.push_back({x, y});
    }

    return pts;
}
