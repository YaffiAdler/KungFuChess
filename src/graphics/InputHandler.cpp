#include "InputHandler.h"
#include "../logic/Controller/Controller.h"
#include <iostream>

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
InputHandler::InputHandler(Controller& controller) noexcept
    : m_controller(controller)
{}

// ─────────────────────────────────────────────
//  register_click — נקרא מ-mouse callback
// ─────────────────────────────────────────────
void InputHandler::register_click(int x, int y) {
    m_clickX   = x;
    m_clickY   = y;
    m_hasClick = true;
}

// ─────────────────────────────────────────────
//  process_click — delegate ל-Controller
// ─────────────────────────────────────────────
bool InputHandler::process_click() {
    if (!m_hasClick) return false;
    m_hasClick = false;

    std::string msg = m_controller.handle_click(m_clickX, m_clickY);
    if (!msg.empty() && msg != "ok" && msg != "selected" && msg != "deselected") {
        std::cout << "Move failed: " << msg << std::endl;
    }

    return true;
}

// ─────────────────────────────────────────────
//  process_key — delegate כל המקשים ל-Controller
// ─────────────────────────────────────────────
bool InputHandler::process_key(int key) {
    if (key == -1) return true;  // אין מקש — המשך לולאה
    return m_controller.handle_key(key);
}

// ─────────────────────────────────────────────
//  flush — איפוס מצב קליקים
// ─────────────────────────────────────────────
void InputHandler::flush() {
    m_hasClick = false;
    m_clickX   = -1;
    m_clickY   = -1;
}
