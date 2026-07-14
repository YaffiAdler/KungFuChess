#include "InputHandler.h"
#include <iostream>

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
InputHandler::InputHandler(const PixelMapper& mapper)
    : m_mapper(mapper)
{}

// ─────────────────────────────────────────────
//  register_click
// ─────────────────────────────────────────────
void InputHandler::register_click(int x, int y) {
    m_clickX.store(x);
    m_clickY.store(y);
    m_hasClick.store(true);
}

// ─────────────────────────────────────────────
//  process_click — עיבוד קליק → פעולת משחק
// ─────────────────────────────────────────────
bool InputHandler::process_click(GameEngine& engine) {
    if (!m_hasClick.exchange(false)) return false;

    int x = m_clickX.exchange(-1);
    int y = m_clickY.exchange(-1);

    // במצב Waiting — מתעלמים מקליקים
    if (engine.state() != GameState::Playing) return true;

    auto cell = m_mapper.to_cell(x, y,
                                  engine.board().rows(),
                                  engine.board().cols());
    if (!cell.has_value()) return true;  // מחוץ ללוח

    auto result = engine.move_selected_to(*cell);
    if (!result.success && !result.message.empty()) {
        std::cout << "Move failed: " << result.message << std::endl;
    }

    return true;
}

// ─────────────────────────────────────────────
//  process_key — עיבוד מקש מקלדת
// ─────────────────────────────────────────────
bool InputHandler::process_key(int key, GameEngine& engine) {
    // ESC או 'q' — יציאה
    if (key == 27 || key == 'q' || key == 'Q') {
        return false;  // signal to exit loop
    }

    // ENTER — התחלת משחק
    if (key == 13 && engine.state() == GameState::Waiting) {
        engine.start_game();
    }

    return true;  // continue loop
}

// ─────────────────────────────────────────────
//  flush_click
// ─────────────────────────────────────────────
void InputHandler::flush_click() {
    m_hasClick.store(false);
    m_clickX.store(-1);
    m_clickY.store(-1);
}
