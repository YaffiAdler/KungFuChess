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
// handle_click — לוגיקת בחירה מלאה + Arbiter
// ─────────────────────────────────────────────
std::string Controller::handle_click(int x, int y, GameEngine& engine) {
    // ── 0. המשחק פעיל ──
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
        const auto& pieceCell = engine.board().at(*cell);

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

    // קליק על אותו תא — התעלם
    if (*cell == source) {
        return "";
    }

    // בדיקת busy — תנועה פעילה
    if (engine.is_busy()) {
        return "motion_in_progress";
    }

    // בדיקת החלפת בחירה (כלי ידידותי ביעד)
    const auto& sourcePiece = engine.board().at(source);
    const auto& targetPiece = engine.board().at(*cell);
    if (targetPiece.has_value() && sourcePiece.has_value() &&
        targetPiece->get_color() == sourcePiece->get_color()) {
        m_selected = *cell;
        engine.deselect();
        engine.select(*cell);
        return "selected";
    }

    // ── אימות המהלך ──
    auto validation = engine.validate_move(source, *cell);
    if (!validation.is_valid) {
        engine.deselect();
        m_selected.reset();
        return validation.reason;
    }

    // ── התחלת תנועה ב-Arbiter ──
    // 1. engine.start_piece_motion — קובע state=move, מחזיר msPerCell
    int msPerCell = engine.start_piece_motion(source, *cell);
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
    Piece movingPiece = *srcCell;  // העתק

    // 3. חישוב מרחק
    int distance = std::max(
        std::abs(source.row - cell->row),
        std::abs(source.col - cell->col));

    // 4. התחלת תנועה ב-Arbiter
    bool started = m_arbiter.startMotionWithDistance(
        source, *cell,
        std::move(movingPiece),
        distance, msPerCell
    );

    if (!started) {
        // לא אמור לקרות — busy כבר נקבע
        engine.deselect();
        m_selected.reset();
        return "arbiter_busy";
    }

    // 5. ניקוי בחירה
    engine.deselect();
    m_selected.reset();

    return "ok";
}

// ─────────────────────────────────────────────
// handle_key — מקשים: ESC, ENTER, q
// ─────────────────────────────────────────────
bool Controller::handle_key(int key, GameEngine& engine) {
    if (key == -1) return true;  // אין מקש — המשך לולאה

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
// tick — קידום state machines + Arbiter, commit בהגעה
// ─────────────────────────────────────────────
void Controller::tick(int deltaMs, GameEngine& engine) {
    // 1. State machines (long_rest/short_rest → idle)
    engine.tick_state_machines(deltaMs);

    // 2. Arbiter motion
    if (!m_arbiter.hasActiveMotion()) {
        return;
    }

    ArbiterTickResult result = m_arbiter.tick(deltaMs);

    if (result.completed) {
        const auto& motion = *m_arbiter.motion();
        engine.commit_move_with_state(motion.from, motion.to);
        m_arbiter.setMotion(std::nullopt);
    }
}
