#include "GameEngine.h"
#include "RuleEngine.h"

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
GameEngine::GameEngine(Board board, GameConfig config)
    : m_board(std::move(board)), m_config(config) {}

// ─────────────────────────────────────────────
//  select — בחירת כלי
// ─────────────────────────────────────────────
bool GameEngine::select(Position pos) {
    if (!m_board.is_valid_position(pos)) return false;

    const auto& cell = m_board.at(pos);
    if (!cell.has_value()) return false;

    if (cell->get_color() != m_currentTurn) return false;

    m_selectedPos = pos;
    return true;
}

// ─────────────────────────────────────────────
//  request_move — תיאום: game_over ← אימות ← ביצוע
// ─────────────────────────────────────────────
MoveResult GameEngine::request_move(Position from, Position to) {
    // ── 0. סיום משחק — דחה לפני RuleEngine ──
    if (m_gameOver) {
        return {false, "game_over"};
    }

    // ── 1. אימות חוקיות — האצלה ל-RuleEngine ──
    RuleEngine ruleEngine(m_board);
    auto validation = ruleEngine.validate_move(from, to);

    if (!validation.is_valid) {
        return {false, validation.reason};
    }

    // ── 2. אימות תור — GameEngine מנהל תורות, לא RuleEngine ──
    const auto& fromCell = m_board.at(from);
    if (fromCell->get_color() != m_currentTurn) {
        return {false, "not_your_turn"};
    }

    // ── 3. בצע מהלך ──
    const auto& targetCell = m_board.at(to);
    bool wasCapture = targetCell.has_value();  // כלי אויב ביעד
    Piece movingPiece = *fromCell;

    m_board.remove(from);
    movingPiece.set_pos(to);
    movingPiece.mark_moved();
    m_board.place(std::move(movingPiece));

    // ✦ הבחירה נשארת על הכלי!
    m_selectedPos = to;

    // ── 4. בדוק תנאי סיום (מלך הוכה) ──
    bool gameOver = false;
    if (wasCapture) {
        bool whiteKingFound = false;
        bool blackKingFound = false;
        for (int r = 0; r < m_board.rows(); ++r) {
            for (int c = 0; c < m_board.cols(); ++c) {
                const auto& cell = m_board.at(r, c);
                if (cell.has_value() && cell->type_id() == "king") {
                    if (cell->get_color() == PieceColor::White) whiteKingFound = true;
                    else blackKingFound = true;
                }
            }
        }
        if (!whiteKingFound || !blackKingFound) {
            gameOver = true;
            m_gameOver = true;
            m_winner = !whiteKingFound ? PieceColor::Black : PieceColor::White;
        }
    }

    // ── 5. העבר תור ──
    m_currentTurn = (m_currentTurn == PieceColor::White)
                    ? PieceColor::Black
                    : PieceColor::White;

    return {true, "ok", wasCapture, gameOver};
}

// ─────────────────────────────────────────────
//  move_selected_to — עוטף request_move עם לוגיקת בחירה
// ─────────────────────────────────────────────
MoveResult GameEngine::move_selected_to(Position target) {
    // ── 0. אין בחירה ──
    if (!m_selectedPos.has_value()) {
        return {false, "No piece selected"};
    }

    Position from = *m_selectedPos;
    const auto& fromCell = m_board.at(from);

    // שמירה: בחירה מצביעה על תא ריק ← בטל
    if (!fromCell.has_value()) {
        m_selectedPos.reset();
        return {false, ""};
    }

    // ── 1. קליק על אותו תא — התעלם ──
    if (target == from) {
        return {false, ""};
    }

    // ── 2. כלי ידידותי ביעד ← החלף בחירה (לפני RuleEngine —
    //        זה לא מהלך, זה שינוי בחירה) ──
    PieceColor selectedColor = fromCell->get_color();
    const auto& targetCell = m_board.at(target);
    if (targetCell.has_value() && targetCell->get_color() == selectedColor) {
        m_selectedPos = target;
        return {true, ""};
    }

    // ── 3. האצלה ל-request_move ──
    return request_move(from, target);
}

// ─────────────────────────────────────────────
//  snapshot
// ─────────────────────────────────────────────
GameSnapshot GameEngine::snapshot() const {
    return GameSnapshot{
        m_board,            // העתקה עמוקה
        m_selectedPos,
        m_gameClock,
        m_currentTurn
    };
}

// ─────────────────────────────────────────────
//  restore
// ─────────────────────────────────────────────
void GameEngine::restore(const GameSnapshot& snap) {
    m_board        = snap.board;
    m_selectedPos  = snap.selectedPos;
    m_gameClock    = snap.gameClock;
    m_currentTurn  = snap.currentTurn;
    m_gameOver     = false;
    m_winner.reset();
}
