#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <fstream>

#include "tokenizer.h"
#include "parser.h"
#include "evaluator.h"

static const int G_W = 100;
static const int G_H = 30;

// ─────────────────────────────────────────────
bool contains(const std::string& expr, const std::vector<std::string>& keys) {
    for (const auto& k : keys)
        if (expr.find(k) != std::string::npos) return true;
    return false;
}

// ─────────────────────────────────────────────
void asciiPlot(const std::vector<Point>& pts,
               const std::string& expr,
               double xMin, double xMax) {

    if (pts.empty()) {
        std::cout << "  [No plottable points]\n\n";
        return;
    }

    // Find Y range
    double yMin = pts.front().y, yMax = pts.front().y;
    for (const auto& p : pts) {
        yMin = std::min(yMin, p.y);
        yMax = std::max(yMax, p.y);
    }

    // 🔥 Symmetric + Clamped scaling
    double absMax = std::max(std::abs(yMin), std::abs(yMax));
    double Y_LIMIT = 50.0;                     // adjust this if needed
    absMax = std::min(absMax, Y_LIMIT);

    yMin = -absMax;
    yMax = absMax;

    // Padding
    double yPad = (yMax - yMin) * 0.05;
    if (std::abs(yMax - yMin) < 1e-10) yPad = 1.0;
    yMin -= yPad;
    yMax += yPad;

    std::vector<std::string> grid(G_H, std::string(G_W, ' '));

    // Y-axis
    int axisCol = -1;
    if (xMin <= 0 && xMax >= 0) {
        axisCol = (int)((0.0 - xMin) / (xMax - xMin) * (G_W - 1));
        axisCol = std::max(0, std::min(G_W - 1, axisCol));
        for (int r = 0; r < G_H; ++r)
            grid[r][axisCol] = '|';
    }

    // X-axis
    if (yMin <= 0 && yMax >= 0) {
        int axisRow = G_H - 1 - (int)((0.0 - yMin) / (yMax - yMin) * (G_H - 1));
        axisRow = std::max(0, std::min(G_H - 1, axisRow));
        for (int c = 0; c < G_W; ++c)
            grid[axisRow][c] = (grid[axisRow][c] == '|') ? '+' : '-';
    }

    // Plot points (with clipping)
    for (const auto& p : pts) {
        if (p.y > yMax || p.y < yMin) continue;

        int col = (int)((p.x - xMin) / (xMax - xMin) * (G_W - 1));
        int row = G_H - 1 - (int)((p.y - yMin) / (yMax - yMin) * (G_H - 1));

        col = std::max(0, std::min(G_W - 1, col));
        row = std::max(0, std::min(G_H - 1, row));

        grid[row][col] = '*';
    }

    // Print
    std::cout << "\n  y = " << expr << "\n";
    std::cout << std::string(G_W + 4, '-') << "\n";
    std::cout << "  y\n";

    for (int r = 0; r < G_H; ++r) {
        if (r % 5 == 0) {
            double yVal = yMax - (double)r / (G_H - 1) * (yMax - yMin);
            std::cout << std::setw(9) << std::fixed << std::setprecision(1)
                      << yVal << " |";
        } else {
            std::cout << "          |";
        }
        std::cout << grid[r] << "\n";
    }

    std::cout << "          +";
    std::cout << std::string(G_W, '-') << "\n";

    std::cout << "           ";
    std::vector<std::pair<int,std::string>> labels = {
        {0, std::to_string((int)xMin)},
        {G_W/4, std::to_string((int)(xMin + (xMax-xMin)/4))},
        {G_W/2, "0"},
        {3*G_W/4, std::to_string((int)(xMax - (xMax-xMin)/4))},
        {G_W-1, std::to_string((int)xMax)}
    };

    int prev = 0;
    for (auto& [col, lbl] : labels) {
        std::cout << std::string(col - prev, ' ') << lbl;
        prev = col + lbl.size();
    }
    std::cout << "  x\n";

    std::cout << "\n  points: " << pts.size() << "\n";
    std::cout << std::string(G_W + 4, '-') << "\n\n";
}

// ─────────────────────────────────────────────
void runPipeline(const std::string& expr) {
    try {
        Tokenizer tok;
        Parser par;
        Evaluator ev;

        auto tokens  = tok.tokenize(expr);
        auto postfix = par.toPostfix(tokens);

        double xMin, xMax, step;

        if (contains(expr, {"sin","cos","tan"})) {
            xMin = -2*M_PI; xMax = 2*M_PI; step = 0.01;
        }
        else if (contains(expr, {"exp"})) {
            xMin = -5; xMax = 5; step = 0.05;
        }
        else if (contains(expr, {"log","ln"})) {
            xMin = 0.1; xMax = 10; step = 0.05;
        }
        else {
            xMin = -10; xMax = 10; step = 0.1;
        }

        auto points = ev.generatePoints(postfix, xMin, xMax, step);
        asciiPlot(points, expr, xMin, xMax);

    } catch (const std::exception& e) {
        std::cout << "\n  [ERROR] " << e.what() << "\n\n";
    }
}

// ─────────────────────────────────────────────
int main() {

    std::cout << "🔥 NEW VERSION RUNNING 🔥\n";

    while (true) {
        std::string expr;
        std::cout << "f(x) = ";
        std::getline(std::cin, expr);

        if (expr == "quit" || expr == "exit") break;
        if (expr.empty()) continue;

        runPipeline(expr);
    }

    return 0;
}
