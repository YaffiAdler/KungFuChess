#include "MoveGenerator.h"
#include <iostream>

namespace {
    // בדיקת גבולות — הלוח הוא שמכיר את הגבולות, לא Position.
    [[nodiscard]] inline bool in_bounds(int row, int col, int numRows, int numCols) noexcept {
        return row >= 0 && row < numRows && col >= 0 && col < numCols;
    }
}

std::vector<Position> MoveGenerator::generate(
    const MovementRule& rule,
    Position from, PieceColor color,
    int numRows, int numCols,
    bool hasMoved)
{
    std::vector<Position> moves;

    for (const auto& d : rule.directions) {
        int dr = d.dr;
        int dc = d.dc;

        std::cerr << "DEBUG MoveGen Step: from=(" << from.row << "," << from.col
                  << ") dr=" << dr << " dc=" << dc
                  << " maxSteps=" << rule.maxSteps
                  << " hasMoved=" << hasMoved
                  << " rows=" << numRows << " cols=" << numCols << std::endl;

        // צעד ראשון
        int r1 = from.row + dr;
        int c1 = from.col + dc;
        bool b1 = in_bounds(r1, c1, numRows, numCols);
        std::cerr << "DEBUG MoveGen r1=(" << r1 << "," << c1 << ") in_bounds=" << b1 << std::endl;

        if (b1) {
            moves.push_back(Position{r1, c1});
            // צעד שני — רק אם maxSteps >= 2 והכלי לא זז
            if (rule.maxSteps >= 2 && !hasMoved) {
                int r2 = from.row + 2*dr;
                int c2 = from.col + 2*dc;
                bool b2 = in_bounds(r2, c2, numRows, numCols);
                std::cerr << "DEBUG MoveGen r2=(" << r2 << "," << c2 << ") in_bounds=" << b2 << std::endl;
                if (b2) {
                    moves.push_back(Position{r2, c2});
                }
            } else {
                std::cerr << "DEBUG MoveGen skip double — maxSteps=" << rule.maxSteps
                          << " hasMoved=" << hasMoved << std::endl;
            }
        } else {
            std::cerr << "DEBUG MoveGen r1 out of bounds" << std::endl;
        }
    }

    return moves;
}
