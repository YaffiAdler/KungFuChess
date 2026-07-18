#include "RealTimeArbiter.h"

// ─────────────────────────────────────────────
// startMotion — התחלת תנועה (גרסה קיימת)
// ─────────────────────────────────────────────
bool RealTimeArbiter::startMotion(Position from, Position to,
                                  Piece piece, int msPerCell) {
    if (m_activeMotion.has_value()) return false;

    int distance = chebyshevDistance(from, to);
    int totalMs  = distance * msPerCell;

    m_activeMotion = Motion{from, to, std::move(piece),
                            totalMs, 0, distance, msPerCell};
    return true;
}

// ─────────────────────────────────────────────
// startMotionWithDistance — גרסה עם distance חיצוני
// ─────────────────────────────────────────────
bool RealTimeArbiter::startMotionWithDistance(Position from, Position to,
                                               Piece piece,
                                               int distance, int msPerCell) {
    if (m_activeMotion.has_value()) return false;

    int totalMs = distance * msPerCell;
    m_activeMotion = Motion{from, to, std::move(piece),
                            totalMs, 0, distance, msPerCell};
    return true;
}

// ─────────────────────────────────────────────
// tick — קידום זמן
// ─────────────────────────────────────────────
ArbiterTickResult RealTimeArbiter::tick(int ms) {
    ArbiterTickResult result;

    if (!m_activeMotion.has_value()) {
        return result;  // אין תנועה פעילה — כלום לא קרה
    }

    m_activeMotion->elapsedMs += ms;

    // בדיקת סיום
    if (m_activeMotion->elapsedMs >= m_activeMotion->totalMs) {
        result.completed = true;

        // clamp elapsed ל-totalMs (למקרה ש-tick גדול)
        m_activeMotion->elapsedMs = m_activeMotion->totalMs;
    }

    return result;
}
