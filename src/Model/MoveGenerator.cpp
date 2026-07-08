#include "MoveGenerator.h"

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
            Position one{from.row + dr, from.col + dc};
            if (one.is_valid(numRows, numCols)) {
                moves.push_back(one);
                // צעד שני — רק אם maxSteps >= 2 והכלי לא זז
                if (rule.maxSteps >= 2 && !hasMoved) {
                    Position two{from.row + 2*dr, from.col + 2*dc};
                    if (two.is_valid(numRows, numCols)) {
                        moves.push_back(two);
                    }
                }
            }
            break;
        }

        // ─── Slide: החלקה עד קצה הלוח ───
        case MovePattern::Slide: {
            for (int step = 1; ; ++step) {
                Position t{from.row + dr*step, from.col + dc*step};
                if (!t.is_valid(numRows, numCols)) break;
                moves.push_back(t);
                // Slide ממשיך עד חסימה פיזית — אבל פה אנחנו מייצרים
                // את כל התאים האפשריים. החסימה מטופלת ברמת הלוח/Game.
            }
            break;
        }

        // ─── Jump: קפיצה קבועה ───
        case MovePattern::Jump: {
            Position t{from.row + dr, from.col + dc};
            if (t.is_valid(numRows, numCols)) {
                moves.push_back(t);
            }
            break;
        }
        }
    }

    return moves;
}
