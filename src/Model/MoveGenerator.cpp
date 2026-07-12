#include "MoveGenerator.h"

namespace {
    // בדיקת גבולות — הלוח הוא שמכיר את הגבולות, לא Position.
    // לכן הבדיקה נעשית כפונקציה פנימית כאן.
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

        switch (rule.pattern) {

        // ─── Step: צעד אחד (או שניים לרגלי) ───
        case MovePattern::Step: {
            // צעד ראשון
            int r1 = from.row + dr;
            int c1 = from.col + dc;
            if (in_bounds(r1, c1, numRows, numCols)) {
                moves.push_back(Position{r1, c1});
                // צעד שני — רק אם maxSteps >= 2 והכלי לא זז
                if (rule.maxSteps >= 2 && !hasMoved) {
                    int r2 = from.row + 2*dr;
                    int c2 = from.col + 2*dc;
                    if (in_bounds(r2, c2, numRows, numCols)) {
                        moves.push_back(Position{r2, c2});
                    }
                }
            }
            break;
        }

        // ─── Slide: החלקה עד קצה הלוח ───
        case MovePattern::Slide: {
            for (int step = 1; ; ++step) {
                int r = from.row + dr*step;
                int c = from.col + dc*step;
                if (!in_bounds(r, c, numRows, numCols)) break;
                moves.push_back(Position{r, c});
                // Slide ממשיך עד חסימה פיזית — אבל פה אנחנו מייצרים
                // את כל התאים האפשריים. החסימה מטופלת ברמת הלוח/GameEngine.
            }
            break;
        }

        // ─── Jump: קפיצה קבועה ───
        case MovePattern::Jump: {
            int r = from.row + dr;
            int c = from.col + dc;
            if (in_bounds(r, c, numRows, numCols)) {
                moves.push_back(Position{r, c});
            }
            break;
        }
        }
    }

    return moves;
}
