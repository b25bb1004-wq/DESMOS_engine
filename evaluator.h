#pragma once
#include "parser.h"
#include <vector>

struct Point {
    double x;
    double y;
};

class Evaluator {
public:
    double             evaluate(const std::vector<Token>& postfix, double xVal);
    std::vector<Point> generatePoints(const std::vector<Token>& postfix,
                                      double xMin, double xMax, double step);
};
