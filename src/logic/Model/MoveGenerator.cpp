#include "MoveGenerator.h"

namespace {

[[nodiscard]] inline bool in_bounds(int row, int col, int numRows, int numCols) noexcept {
    return row >= 0 && row < numRows && col >= 0 && col < numCols;
}

/// מקדם כיוון לרגלי: ההגדרות ב-Registry הן בכיוון לבן (למעלה).
/// לבן = 1 (ללא היפוך), שחור = -1 (היפוך כיוון — הולך למטה).
[[nodiscard]] inline int forward(PieceColor color) noexcept {
    return (color == PieceColor::White) ? 1 : -1;
}

} // anonymous

std::vector<Position> MoveGenerator::generate(
    const MovementRule& rule,
    Position from, PieceColor color,
    int numRows, int numCols,
    bool hasMoved)
{
    std::vector<Position> moves;

    switch (rule.pattern) {

    // ── Step: צעד בודד (או שניים לרגלי) ──
    case MovePattern::Step: {
        int fwd = forward(color);
        for (const auto& d : rule.directions) {
            int dr = d.dr * fwd;   // לבן = -1 (למעלה), שחור = +1 (למטה)
            int dc = d.dc;

            // צעד ראשון
            int r1 = from.row + dr;
            int c1 = from.col + dc;
            if (in_bounds(r1, c1, numRows, numCols)) {
                moves.push_back(Position{r1, c1});

                // צעד שני — רק אם maxSteps >= 2 והכלי לא זז
                if (rule.maxSteps >= 2 && !hasMoved) {
                    int r2 = from.row + 2 * dr;
                    int c2 = from.col + 2 * dc;
                    if (in_bounds(r2, c2, numRows, numCols)) {
                        moves.push_back(Position{r2, c2});
                    }
                }
            }
        }
        break;
    }

    // ── Slide: החלקה לאורך קו עד קצה הלוח ──
    case MovePattern::Slide: {
        for (const auto& d : rule.directions) {
            int dr = d.dr;   // Slide לא מתהפך לפי צבע — הכיוונים אבסולוטיים
            int dc = d.dc;
            int r = from.row + dr;
            int c = from.col + dc;
            while (in_bounds(r, c, numRows, numCols)) {
                moves.push_back(Position{r, c});
                r += dr;
                c += dc;
            }
        }
        break;
    }

    // ── Jump: קפיצה קבועה (L של פרש) — לא תלויה בכיוון צבע ──
    case MovePattern::Jump: {
        for (const auto& d : rule.directions) {
            int r = from.row + d.dr;
            int c = from.col + d.dc;
            if (in_bounds(r, c, numRows, numCols)) {
                moves.push_back(Position{r, c});
            }
        }
        break;
    }

    } // switch

    return moves;
}
