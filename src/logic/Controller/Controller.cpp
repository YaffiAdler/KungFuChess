#include "Controller.h"
#include "../Model/GameEngine.h"
#include "../Model/Board.h"

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
Controller::Controller(const GameConfig& config) noexcept
    : m_mapper(config)
{}

// ─────────────────────────────────────────────
// cancel_selection
// ─────────────────────────────────────────────
void Controller::cancel_selection() noexcept {
    m_selected.reset();
}

// ─────────────────────────────────────────────
// handle_click — לוגיקת בחירה מלאה + Arbiter (רב-תנועתי)
// ─────────────────────────────────────────────
std::string Controller::handle_click(int x, int y, GameEngine& engine) {
    // ── 0. המשחק פעיל? ──
    if (engine.state() != GameState::Playing) {
        return "";
    }

    // ── 1. המרת פיקסלים לתא לוח ──
    auto cell = m_mapper.to_cell(x, y,
                                  engine.board().rows(),
                                  engine.board().cols());

    // ── 2. קליק מחוץ ללוח ──
    if (!cell.has_value()) {
        if (m_selected.has_value()) {
            // יש בחירה — בטל אותה
            engine.deselect();
            m_selected.reset();
            return "deselected";
        }
        // אין בחירה — התעלם
        return "";
    }

    // ── 3. קליק בתוך הלוח ──
    if (!m_selected.has_value()) {
        // ── קליק ראשון: ניסיון בחירה ──
        auto& pieceCell = engine.board().at(*cell);

        // התעלם מתאים ריקים
        if (!pieceCell.has_value()) {
            return "";
        }

        // Kung-Fu Chess: אין תורות — בחר כל כלי
        if (engine.select(*cell)) {
            m_selected = *cell;
            return "selected";
        }
        return "";
    }

    // ── קליק שני: ניסיון מהלך ──
    Position source = *m_selected;

    // קליק על אותו תא — קפיצה במקום
    if (source == *cell) {
        if (engine.startJump(source)) {
            m_selected.reset();
            engine.deselect();
            return "jump";
        }
        return "";
    }

    // בדיקת כלי באותו צבע ביעד — החלפת בחירה
    const auto& sourcePiece = engine.board().at(source);
    const auto& targetPiece = engine.board().at(*cell);
    if (targetPiece.has_value() && sourcePiece.has_value() &&
        targetPiece->get_color() == sourcePiece->get_color()) {
        engine.deselect();
        engine.select(*cell);
        m_selected = *cell;
        return "reselect";
    }

    // אימות המהלך
    auto validation = engine.validate_move(source, *cell);
    if (!validation.is_valid) {
        engine.deselect();
        m_selected.reset();
        return validation.reason;
    }

    // ── התחלת תנועה ב-Arbiter (ללא הגבלת busy) ──
    // 1. engine.startMotion — קובע state=move, מחזיר msPerCell
    int msPerCell = engine.startMotion(source, *cell);
    if (msPerCell <= 0) {
        engine.deselect();
        m_selected.reset();
        return "motion_start_failed";
    }

    // 2. העתקת הכלי (המקורי נשאר בלוח עם state=move)
    const auto& srcCell = engine.board().at(source);
    if (!srcCell.has_value()) {
        engine.deselect();
        m_selected.reset();
        return "source_empty";
    }
    Piece movingPiece = *srcCell; // העתק

    // 3. חישוב מרחק
    int distance = std::max(
        std::abs(source.row - cell->row),
        std::abs(source.col - cell->col));

    // 4. התחלת תנועה ב-Arbiter
    m_arbiter.startMotionWithDistance(
        source, *cell,
        std::move(movingPiece),
        distance, msPerCell
    );

    // 5. ניקוי בחירה
    engine.deselect();
    m_selected.reset();

    return "ok";
}

// ─────────────────────────────────────────────
// handle_key — מקשים: ESC, ENTER, q
// ─────────────────────────────────────────────
bool Controller::handle_key(int key, GameEngine& engine) {
    if (key == -1) return true; // אין מקש — המשך לולאה

    // ESC או q — צא
    if (key == 27 || key == 'q' || key == 'Q') {
        return false;
    }

    // ENTER — התחלה / הפעלה מחדש (מ-GameOver)
    if (key == 13) {
        if (engine.state() == GameState::Waiting) {
            engine.start_game();
        }
        return true;
    }

    return true; // כל מקש אחר — המשך לולאה
}

// ─────────────────────────────────────────────
// tick — האצלה מלאה ל-GameEngine
// ─────────────────────────────────────────────
void Controller::tick(int deltaMs, GameEngine& engine) {
    engine.tick(deltaMs, m_arbiter);
}
