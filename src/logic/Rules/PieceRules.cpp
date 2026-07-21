#include "PieceRules.h"
#include "../Model/PieceTypeRegistry.h"
#include "MovementRule.h"
#include <cstdlib> // abs

const char* PieceRules::m_lastReason = nullptr;

// ─────────────────────────────────────────────
//  slide_path_clear
// ─────────────────────────────────────────────
bool PieceRules::slide_path_clear(const Board& board, Position from, Position to) {
    int dr = to.row - from.row;
    int dc = to.col - from.col;

    int stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
    int stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);

    int r = from.row + stepR;
    int c = from.col + stepC;

    while (r != to.row || c != to.col) {
        const auto& cell = board.at(r, c);
        if (cell.has_value()) {
            m_lastReason = "blocked_path";
            return false;
        }
        r += stepR;
        c += stepC;
    }
    return true;
}

// ─────────────────────────────────────────────
//  step_path_clear
// ─────────────────────────────────────────────
bool PieceRules::step_path_clear(const Board& board, Position from, Position to, int maxSteps) {
    int dr = to.row - from.row;
    int dc = to.col - from.col;

    int absDr = std::abs(dr);
    int absDc = std::abs(dc);

    // צעד בודד — אין תאי ביניים
    if (absDr <= 1 && absDc <= 1) {
        return true;
    }

    // צעד כפול (רגלי) — בדוק תא אמצעי
    if (maxSteps >= 2 && (absDr == 2 || absDc == 2) && (absDr <= 2 && absDc <= 2)) {
        int midR = from.row + dr / 2;
        int midC = from.col + dc / 2;
        const auto& midCell = board.at(midR, midC);
        if (midCell.has_value()) {
            m_lastReason = "blocked_step";
            return false;
        }
        return true;
    }

    m_lastReason = "illegal_piece_move";
    return false;
}

// ─────────────────────────────────────────────
//  dir_match — בודק התאמת כיוון (בלי capture logic)
// ─────────────────────────────────────────────
[[nodiscard]] static bool dir_match(int dr, int dc,
                                     const Direction& d,
                                     MovePattern pattern, int maxSteps) noexcept
{
    switch (pattern) {
    case MovePattern::Slide: {
        int signR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
        int signC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
        int expSignR = (d.dr == 0) ? 0 : (d.dr > 0 ? 1 : -1);
        int expSignC = (d.dc == 0) ? 0 : (d.dc > 0 ? 1 : -1);
        return signR == expSignR && signC == expSignC &&
               (dr == 0) == (d.dr == 0) && (dc == 0) == (d.dc == 0);
    }
    case MovePattern::Step: {
        // registry מגדיר dir=למעלה. שחור = dr הפוך.
        if (dr == d.dr && dc == d.dc) return true;
        if (dr == -d.dr && dc == d.dc) return true;
        if (maxSteps >= 2 && dr == 2 * d.dr && dc == 2 * d.dc) return true;
        if (maxSteps >= 2 && dr == -2 * d.dr && dc == 2 * d.dc) return true;
        return false;
    }
    case MovePattern::Jump:
        return dr == d.dr && dc == d.dc;
    }
    return false;
}

// ─────────────────────────────────────────────
//  is_path_clear — ONLY checks blocking
// ─────────────────────────────────────────────
bool PieceRules::is_path_clear(const Board& board, const Piece& piece,
                                Position from, Position to) noexcept
{
    m_lastReason = nullptr;

    int dr = to.row - from.row;
    int dc = to.col - from.col;
    if (dr == 0 && dc == 0) return true;

    const auto* def = PieceTypeRegistry::instance().find_by_id(piece.type_id());
    if (!def) { m_lastReason = "illegal_piece_move"; return false; }

    for (auto& rule : def->rules) {
        for (const auto& d : rule.directions) {
            if (!dir_match(dr, dc, d, rule.pattern, rule.maxSteps)) continue;

            // found matching rule — check blocking
            switch (rule.pattern) {
            case MovePattern::Slide: return slide_path_clear(board, from, to);
            case MovePattern::Step:  return step_path_clear(board, from, to, rule.maxSteps);
            case MovePattern::Jump:  return true;
            }
        }
    }

    m_lastReason = "illegal_piece_move";
    return false;
}
