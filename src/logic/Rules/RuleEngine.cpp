#include "RuleEngine.h"
#include "PieceRules.h"
#include "../Model/PieceTypeRegistry.h"
#include <algorithm>
#include <vector>

// ─────────────────────────────────────────────
// check_game_over — בדיקת סיום: איזה צבע איבד מלך
// ─────────────────────────────────────────────
std::optional<PieceColor> RuleEngine::check_game_over(const Board& board) noexcept {
    bool whiteKingFound = false;
    bool blackKingFound = false;

    for (int r = 0; r < board.rows(); ++r) {
        for (int c = 0; c < board.cols(); ++c) {
            const auto& cell = board.at(r, c);
            if (cell.has_value() && cell->type_id() == "king") {
                if (cell->get_color() == PieceColor::White) whiteKingFound = true;
                else blackKingFound = true;
            }
        }
    }

    if (!whiteKingFound) return PieceColor::Black;
    if (!blackKingFound) return PieceColor::White;
    return std::nullopt;
}

namespace {

struct RuleMatch {
    const MovementRule* rule = nullptr;
    bool                canCapture = false;
};

/// מוצא את כל הכללים שמתאימים לכיוון המהלך
[[nodiscard]] std::vector<RuleMatch> find_matching_rules(
    const Piece& piece, int dr, int dc, int maxSteps)
{
    std::vector<RuleMatch> result;
    const auto* def = PieceTypeRegistry::instance().find_by_id(piece.type_id());
    if (!def) return result;

    for (auto& rule : def->rules) {
        for (const auto& d : rule.directions) {
            bool match = false;
            switch (rule.pattern) {
            case MovePattern::Slide: {
                int signR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
                int signC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
                int expSignR = (d.dr == 0) ? 0 : (d.dr > 0 ? 1 : -1);
                int expSignC = (d.dc == 0) ? 0 : (d.dc > 0 ? 1 : -1);
                match = signR == expSignR && signC == expSignC &&
                        (dr == 0) == (d.dr == 0) && (dc == 0) == (d.dc == 0);
                break;
            }
            case MovePattern::Step:
                match = (dr == d.dr && dc == d.dc) ||
                        (dr == -d.dr && dc == d.dc) ||
                        (maxSteps >= 2 && dr == 2 * d.dr && dc == 2 * d.dc) ||
                        (maxSteps >= 2 && dr == -2 * d.dr && dc == 2 * d.dc);
                break;
            case MovePattern::Jump:
                match = dr == d.dr && dc == d.dc;
                break;
            }
            if (match) {
                result.push_back({&rule, rule.canCapture});
            }
        }
    }
    return result;
}

} // anonymous

MoveValidation RuleEngine::validate_move(Position from, Position to) const noexcept {
    // ── 1. גבולות ──
    if (!m_board->is_valid_position(from)) return {false, "outside_board"};
    if (!m_board->is_valid_position(to))   return {false, "outside_board"};

    // ── 2. מקור ריק ──
    const auto& srcCell = m_board->at(from);
    if (!srcCell.has_value()) return {false, "empty_source"};

    const Piece& piece = *srcCell;

if (piece.get_state() == PieceState::short_rest || piece.get_state() == PieceState::long_rest)
    return {false, "piece_at_rest"};

    // ── 3. לאותו תא ──
    if (from == to) return {false, "illegal_piece_move"};

    // ── 4. friendly_destination ──
    const auto& dstCell = m_board->at(to);
    bool isCapture = dstCell.has_value();
    if (isCapture && dstCell->get_color() == piece.get_color())
        return {false, "friendly_destination"};

    // ── 5. גאומטריה (MoveGenerator) ──
    auto rawMoves = piece.get_raw_moves(m_board->rows(), m_board->cols());
    if (std::find(rawMoves.begin(), rawMoves.end(), to) == rawMoves.end())
        return {false, "illegal_piece_move"};

    // ── 6. canCapture — RuleEngine ──
    int dr = to.row - from.row;
    int dc = to.col - from.col;
    int maxSteps = 1;
    {
        const auto* def = PieceTypeRegistry::instance().find_by_id(piece.type_id());
        if (def) {
            for (auto& rule : def->rules)
                if (rule.maxSteps > maxSteps) maxSteps = rule.maxSteps;
        }
    }

    auto matchingRules = find_matching_rules(piece, dr, dc, maxSteps);

    // בדוק האם יש כלל canCapture=false וגם כלל canCapture=true (מצב רגלי)
    bool hasNoCaptureRule = false;
    bool hasCaptureRule   = false;
    for (auto& mr : matchingRules) {
        if (mr.canCapture) hasCaptureRule = true;
        else               hasNoCaptureRule = true;
    }

    if (isCapture && hasNoCaptureRule && !hasCaptureRule) {
        // רק כלל ללא הכאה — אי אפשר להכות (רגלי קדימה אל אויב)
        return {false, "illegal_piece_move"};
    }
    // רגלי אלכסון לריק: יש גם כלל הכאה וגם כלל ללא הכאה, אבל התנועה אלכסונית
    // ולכן ה-matched-rule היחיד הוא canCapture=true
    bool isPawn = (piece.type_id() == "pawn");  // רגלי — הכלי היחיד עם כללי הכאה/ללא-הכאה נפרדים
    if (!isCapture && isPawn && hasCaptureRule && !hasNoCaptureRule) {
        // רגלי: מהלך אלכסוני לריק — אסור
        return {false, "illegal_piece_move"};
    }

    // ── 7. חסימות — PieceRules ──
    if (!PieceRules::is_path_clear(*m_board, piece, from, to))
        return {false, PieceRules::last_block_reason()};
 
    return {true, "ok"};
}
