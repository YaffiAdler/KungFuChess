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
std::string Controller::handle_click(int x, int y) {
    if (!m_engine) return "";

    // ── 0. המשחק פעיל? ──
    if (m_engine->state() != GameState::Playing) {
        return "";
    }

    // ── 1. המרת פיקסלים לתא לוח ──
    auto cell = m_mapper.to_cell(x, y,
                                  m_engine->board().rows(),
                                  m_engine->board().cols());

    // ── 2. קליק מחוץ ללוח ──
    if (!cell.has_value()) {
        if (m_selected.has_value()) {
            // יש בחירה — בטל אותה
            m_engine->deselect();
            m_selected.reset();
            return "deselected";
        }
        // אין בחירה — התעלם
        return "";
    }

    // ── 3. קליק בתוך הלוח ──
    if (!m_selected.has_value()) {
        // ── קליק ראשון: ניסיון בחירה ──
        auto& pieceCell = m_engine->board().at(*cell);

        // התעלם מתאים ריקים
        if (!pieceCell.has_value()) {
            return "";
        }

        // Kung-Fu Chess: אין תורות — בחר כל כלי
        if (m_engine->select(*cell)) {
            m_selected = *cell;
            return "selected";
        }
        return "";
    }

    // ── קליק שני: ניסיון מהלך ──
    Position source = *m_selected;

    // קליק על אותו תא — קפיצה במקום
    if (source == *cell) {
        if (m_engine->startJump(source)) {
            m_selected.reset();
            m_engine->deselect();
            return "jump";
        }
        return "";
    }

    // בדיקת כלי באותו צבע ביעד — החלפת בחירה
    const auto& sourcePiece = m_engine->board().at(source);
    const auto& targetPiece = m_engine->board().at(*cell);
    if (targetPiece.has_value() && sourcePiece.has_value() &&
        targetPiece->get_color() == sourcePiece->get_color()) {
        m_engine->deselect();
        m_engine->select(*cell);
        m_selected = *cell;
        return "reselect";
    }

    // אימות המהלך
    auto validation = m_engine->validate_move(source, *cell);
    if (!validation.is_valid) {
        m_engine->deselect();
        m_selected.reset();
        return validation.reason;
    }

    // ── התחלת תנועה ב-Arbiter (ללא הגבלת busy) ──
    // 1. engine.startMotion — קובע state=move, מחזיר msPerCell
    int msPerCell = m_engine->startMotion(source, *cell);
    if (msPerCell <= 0) {
        m_engine->deselect();
        m_selected.reset();
        return "motion_start_failed";
    }

    // 2. העתקת הכלי (המקורי נשאר בלוח עם state=move)
    const auto& srcCell = m_engine->board().at(source);
    if (!srcCell.has_value()) {
        m_engine->deselect();
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
    m_engine->deselect();
    m_selected.reset();

    return "ok";
}

// ─────────────────────────────────────────────
// handle_key — מקשים: ESC, ENTER, q
// ─────────────────────────────────────────────
bool Controller::handle_key(int key) {
    if (!m_engine) return true;

    if (key == -1) return true; // אין מקש — המשך לולאה

    // ESC או q — צא
    if (key == 27 || key == 'q' || key == 'Q') {
        return false;
    }

    // ENTER — התחלה / הפעלה מחדש (מ-GameOver)
    if (key == 13) {
        if (m_engine->state() == GameState::Waiting) {
            m_engine->start_game();
        }
        return true;
    }

    return true; // כל מקש אחר — המשך לולאה
}

// ─────────────────────────────────────────────
// tick — האצלה מלאה ל-GameEngine
// ─────────────────────────────────────────────
void Controller::tick(int deltaMs) {
    if (m_engine) {
        m_engine->tick(deltaMs, m_arbiter);
    }
}
