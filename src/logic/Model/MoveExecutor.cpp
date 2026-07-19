#include "MoveExecutor.h"
#include "RuleEngine.h"
#include "PromotionHandler.h"

// ─────────────────────────────────────────────
// execute — ביצוע מהלך לוגי על הלוח בלבד
// ─────────────────────────────────────────────
MoveExecResult MoveExecutor::execute(Position from, Position to) {
    // ── 1. קריאת הכלי מהמקור ──
    auto& fromCell = m_board->at(from);
    if (!fromCell.has_value()) {
        return {false, false, false, std::nullopt};
    }

    Piece movingPiece = std::move(*fromCell);
    m_board->remove(from);

    // ── 2. בדיקת הכאה ──
    auto& targetCell = m_board->at(to);
    bool wasCapture = targetCell.has_value();

    // ── 3. הצבת הכלי ביעד ──
    movingPiece.set_pos(to);
    movingPiece.mark_moved();

    // ── 4. קידום רגלי (promotion אוטומטי למלכה) ──
    movingPiece = try_promote_pawn(std::move(movingPiece), m_board->rows());

    m_board->place(std::move(movingPiece));

    // ── 5. בדיקת סיום (מלך הוכה) ──
    bool gameOver = false;
    std::optional<PieceColor> winner;
    if (wasCapture) {
        winner = RuleEngine::check_game_over(*m_board);
        if (winner.has_value()) {
            gameOver = true;
        }
    }

    // Kung-Fu Chess: אין תורות
    return {true, wasCapture, gameOver, winner};
}
