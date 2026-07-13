#include "PieceRules.h"
#include "PieceTypeRegistry.h"
#include "MovementRule.h"
#include <cstdlib> // abs
#include <iostream>

const char* PieceRules::m_lastReason = nullptr;

// ─────────────────────────────────────────────
//  slide_path_clear — בדיקת חסימה ל-Slide
// ─────────────────────────────────────────────
bool PieceRules::slide_path_clear(const Board& board, Position from, Position to) {
    int dr = to.row - from.row;
    int dc = to.col - from.col;

    // נרמול כיוון (1, -1, או 0)
    int stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
    int stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);

    // צעד תא-תא, עד התא שלפני היעד
    int r = from.row + stepR;
    int c = from.col + stepC;

    while (r != to.row || c != to.col) {
        const auto& cell = board.at(r, c);
        if (cell.has_value()) {
            // תא ביניים תפוס — חסום!
            m_lastReason = "blocked_path";
            return false;
        }
        r += stepR;
        c += stepC;
    }

    // הגענו ל-to — התא הזה יכול להיות ריק או אויב (RuleEngine יבדוק friendly_destination)
    return true;
}

// ─────────────────────────────────────────────
//  step_path_clear — בדיקת חסימה ל-Step
// ─────────────────────────────────────────────
bool PieceRules::step_path_clear(const Board& board, Position from, Position to, int maxSteps) {
    int dr = to.row - from.row;
    int dc = to.col - from.col;

    int absDr = std::abs(dr);
    int absDc = std::abs(dc);

    // צעד בודד — אין תאי ביניים לבדוק
    if (absDr <= 1 && absDc <= 1) {
        return true;
    }

    // צעד כפול (רגלי) — בדיקת שורת התחלה + תא אמצעי
    if (maxSteps >= 2 && (absDr == 2 || absDc == 2) && (absDr <= 2 && absDc <= 2)) {
        // ── בדוק שורת התחלה ──
        int startRow = -1;
        const auto& srcCell = board.at(from);
        if (srcCell.has_value()) {
            PieceColor c = srcCell->get_color();
            startRow = (c == PieceColor::White) ? (board.rows() - 2) : 1;
        }
        if (startRow != -1 && from.row != startRow) {
            m_lastReason = "illegal_piece_move";
            return false;
        }

        // ── בדוק תא אמצעי ──
        int midR = from.row + dr / 2;
        int midC = from.col + dc / 2;
        const auto& midCell = board.at(midR, midC);
        if (midCell.has_value()) {
            m_lastReason = "blocked_step";
            return false;
        }
        return true;
    }

    // צעד גדול יותר — לא נתמך
    m_lastReason = "illegal_piece_move";
    return false;
}

// ─────────────────────────────────────────────
//  is_path_clear — נקודת כניסה ראשית
// ─────────────────────────────────────────────
bool PieceRules::is_path_clear(const Board& board, const Piece& piece,
                                Position from, Position to,
                                const MovementRule** matched_rule) noexcept
{
    m_lastReason = nullptr;

    int dr = to.row - from.row;
    int dc = to.col - from.col;

    if (dr == 0 && dc == 0) {
        return true; // מהלך לעצמו — RuleEngine יידחה
    }

    // ── השג את כללי התנועה של הכלי מה-Registry ──
    const auto* def = PieceTypeRegistry::instance().find_by_id(piece.type_id());
    if (!def) {
        m_lastReason = "illegal_piece_move";
        return false;
    }

    // ── חפש איזה MovementRule מתאים לכיוון המהלך ──
    std::cerr << "DEBUG is_path_clear: piece=" << piece.to_token()
              << " from=" << from.row << "," << from.col
              << " to=" << to.row << "," << to.col
              << " dr=" << dr << " dc=" << dc
              << " def->rules.size=" << def->rules.size() << std::endl;
    for (auto& rule : def->rules) {
        std::cerr << "DEBUG checking rule: pattern=" << static_cast<int>(rule.pattern)
                  << " maxSteps=" << rule.maxSteps
                  << " directions.size=" << rule.directions.size() << std::endl;
        for (const auto& d : rule.directions) {
            std::cerr << "DEBUG direction: d.dr=" << d.dr << " d.dc=" << d.dc << std::endl;
            // בדוק שהכיוון תואם
            int expectedDr = d.dr;
            int expectedDc = d.dc;

            bool match = false;

            switch (rule.pattern) {
            case MovePattern::Slide: {
                // Slide: dr ו-dc צריכים להיות באותו כיוון כמו d
                int signR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
                int signC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
                int expSignR = (expectedDr == 0) ? 0 : (expectedDr > 0 ? 1 : -1);
                int expSignC = (expectedDc == 0) ? 0 : (expectedDc > 0 ? 1 : -1);
                if (signR == expSignR && signC == expSignC &&
                    (dr == 0) == (expectedDr == 0) &&
                    (dc == 0) == (expectedDc == 0))
                    match = true;
                break;
            }

            case MovePattern::Step: {
                // Step: dr/dc צריכים להתאים בדיוק, או להיות כפולה (2 צעדים)
                // הכיוון ב-PieceTypeRegistry הוא תמיד "למעלה" (dr שלילי).
                // MoveGenerator לא הופך כיוונים — הוא משתמש ב-dr/dc כפי שהם.
                // לכן:
                //  - רגלי לבן: dr אמור להיות שלילי (match ל-expectedDr)
                //  - רגלי שחור: dr אמור להיות חיובי (match ל-expectedDr*-1)
                if (dr == expectedDr && dc == expectedDc) {
                    match = true;
                } else if (dr == -expectedDr && dc == expectedDc) {
                    // רגלי שחור — dr הפוך
                    match = true;
                } else if (rule.maxSteps >= 2 && dr == 2 * expectedDr && dc == 2 * expectedDc) {
                    // צעד כפול לבן
                    match = true;
                } else if (rule.maxSteps >= 2 && dr == -2 * expectedDr && dc == 2 * expectedDc) {
                    // צעד כפול שחור
                    match = true;
                }
                break;
            }

            case MovePattern::Jump: {
                // Jump: dr/dc צריכים להתאים בדיוק (לבן) או הפוך (שחור)
                if (dr == expectedDr && dc == expectedDc) {
                    match = true;
                }
                break;
            }
            }

            if (!match) { std::cerr << "DEBUG no match, continue" << std::endl; continue; }

            std::cerr << "DEBUG match! rule pattern=" << static_cast<int>(rule.pattern) << std::endl;
            if (matched_rule) {
                *matched_rule = &rule;
            }

            // ── בדוק חסימות לפי סוג התנועה ──
            switch (rule.pattern) {
            case MovePattern::Slide:
                return slide_path_clear(board, from, to);

            case MovePattern::Step:
                return step_path_clear(board, from, to, rule.maxSteps);

            case MovePattern::Jump:
                // Knight קופץ מעל — אף פעם לא חסום
                return true;
            }
        }
    }

    // לא נמצא כלל תואם — המהלך לא חוקי
    m_lastReason = "illegal_piece_move";
    return false;
}
