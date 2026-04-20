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


// ─────────────────────────────────────────────
//  FIXED SETTINGS
//  Range : -100 to 100
//  Step  : 0.5  → 400 points, smooth curve
//  Graph : 100 wide × 30 tall
// ─────────────────────────────────────────────
static const double X_MIN  = -100.0;
static const double X_MAX  =  100.0;
static const double STEP   =    0.5;
static const int    G_W    =   100;   // graph width  (chars)
static const int    G_H    =    30;   // graph height (chars)

// ─────────────────────────────────────────────
//  ASCII GRAPH
// ─────────────────────────────────────────────
void asciiPlot(const std::vector<Point>& pts, const std::string& expr) {
    if (pts.empty()) {
        std::cout << "  [No plottable points — expression undefined over entire range]\n\n";
        return;
    }

    // ── Find bounds ───────────────────────────
    double yMin = pts.front().y, yMax = pts.front().y;
    for (const auto& p : pts) {
        yMin = std::min(yMin, p.y);
        yMax = std::max(yMax, p.y);
    }
    // Pad y slightly so points don't sit on the very edge
    double yPad = (yMax - yMin) * 0.05;
    if (std::abs(yMax - yMin) < 1e-10) { yPad = 1.0; }
    yMin -= yPad; yMax += yPad;

    // ── Build empty grid ──────────────────────
    std::vector<std::string> grid(G_H, std::string(G_W, ' '));

    // ── Y-axis  (x = 0) ───────────────────────
    int axisCol = static_cast<int>((0.0 - X_MIN) / (X_MAX - X_MIN) * (G_W - 1));
    axisCol = std::max(0, std::min(G_W - 1, axisCol));
    for (int r = 0; r < G_H; ++r) grid[r][axisCol] = '|';

    // ── X-axis  (y = 0) ───────────────────────
    if (yMin <= 0.0 && yMax >= 0.0) {
        int axisRow = G_H - 1 - static_cast<int>((0.0 - yMin) / (yMax - yMin) * (G_H - 1));
        axisRow = std::max(0, std::min(G_H - 1, axisRow));
        for (int c = 0; c < G_W; ++c)
            grid[axisRow][c] = (grid[axisRow][c] == '|') ? '+' : '-';
    }

    // ── Plot points ───────────────────────────
    for (const auto& p : pts) {
        int col = static_cast<int>((p.x - X_MIN) / (X_MAX - X_MIN) * (G_W - 1));
        int row = G_H - 1 - static_cast<int>((p.y - yMin) / (yMax - yMin) * (G_H - 1));
        col = std::max(0, std::min(G_W - 1, col));
        row = std::max(0, std::min(G_H - 1, row));
        grid[row][col] = '*';
    }

    // ── Print ─────────────────────────────────
    std::cout << "\n";

    // Title bar
    std::string title = "  y = " + expr;
    std::cout << title << "\n";
    std::cout << std::string(G_W + 4, '-') << "\n";

    // Y-axis label on top-right
    std::cout << "  y\n";

    for (int r = 0; r < G_H; ++r) {
        // Y value label on left every 5 rows
        if (r % 5 == 0) {
            double yVal = yMax - (double)r / (G_H - 1) * (yMax - yMin);
            std::cout << std::setw(9) << std::fixed << std::setprecision(1) << yVal << " |";
        } else {
            std::cout << "          |";
        }
        std::cout << grid[r] << "\n";
    }

    // X-axis bottom labels
    std::cout << "          +";
    std::cout << std::string(G_W, '-') << "\n";

    // Print x labels: -100, -50, 0, 50, 100
    std::cout << "           ";
    std::vector<std::pair<int,std::string>> xlabels = {
        {0, "-100"}, {25, "-50"}, {50, "0"}, {75, "50"}, {99, "100"}
    };
    int prev = 0;
    for (auto& [col, lbl] : xlabels) {
        std::cout << std::string(col - prev, ' ') << lbl;
        prev = col + (int)lbl.size();
    }
    std::cout << "  x\n";

    // Stats
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n  y range: [" << yMin + yPad << " → " << yMax - yPad << "]"
              << "   points: " << pts.size() << "\n";
    std::cout << std::string(G_W + 4, '-') << "\n\n";
}

// ─────────────────────────────────────────────
//  PIPELINE
// ─────────────────────────────────────────────
void runPipeline(const std::string& expr) {
    try {
        Tokenizer tok; Parser par; Evaluator ev;

        auto tokens  = tok.tokenize(expr);
        auto postfix = par.toPostfix(tokens);
        auto points  = ev.generatePoints(postfix, X_MIN, X_MAX, STEP);

        std::ofstream file("points.txt");
            for (const auto& p : points) {
                file << p.x << " " << p.y << "\n";
        }
        file.close();

        asciiPlot(points, expr);

    } catch (const std::exception& e) {
        std::cout << "\n  [ERROR] " << e.what() << "\n\n";
    }
}

// ─────────────────────────────────────────────
//  ENTRY POINT
// ─────────────────────────────────────────────
int main() {

    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║              DESMOS ENGINE  —  ICS Project                   ║
╠══════════════════════════════════════════════════════════════╣
║  OPERATORS   : + - * / ^                                     ║
║  CONSTANTS   : pi   e                                        ║
║  TRIG        : sin  cos  tan  asin  acos  atan               ║
║  HYPERBOLIC  : sinh  cosh  tanh                              ║
║  EXP / LOG   : exp  ln  log  log2                            ║
║  ROOTS       : sqrt  cbrt                                    ║
║  MISC        : abs  ceil  floor  sign                        ║
╠══════════════════════════════════════════════════════════════╣
║  EXAMPLES                                                    ║
║   x^2 - 4             sin(3x+2)        exp(-x^2)             ║
║   x^3 - 3x^2 + 2      cos(x)*exp(-x)   1/(x^2+1)             ║
║   x^4 - 5x^2 + 4      ln(abs(x))       x^5 - 4x^3 + 3x       ║
╚══════════════════════════════════════════════════════════════╝
  Range: x = -100 to 100   |   Step: 0.5   |   Type 'quit' to exit
)";

    while (true) {
        std::string expr;
        std::cout << "f(x) = ";
        std::getline(std::cin, expr);

        if (expr == "quit" || expr == "q" || expr == "exit") {
            std::cout << "\nGoodbye!\n\n";
            break;
        }
        if (expr.empty()) continue;

        runPipeline(expr);
    }

    return 0;
}

