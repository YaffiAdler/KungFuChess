#include "RealTimeArbiter.h"
#include <algorithm>

// ─────────────────────────────────────────────
// startMotion — התחלת תנועה (ללא הגבלה)
// ─────────────────────────────────────────────
void RealTimeArbiter::startMotion(Position from, Position to,
                                   Piece piece, int msPerCell) {
    int distance = chebyshevDistance(from, to);
    int totalMs  = distance * msPerCell;

    m_activeMotions.push_back(Motion{from, to, std::move(piece),
                                      totalMs, 0, distance, msPerCell});
}

// ─────────────────────────────────────────────
// startMotionWithDistance — גרסה עם distance חיצוני
// ─────────────────────────────────────────────
void RealTimeArbiter::startMotionWithDistance(Position from, Position to,
                                               Piece piece,
                                               int distance, int msPerCell) {
    int totalMs = distance * msPerCell;
    m_activeMotions.push_back(Motion{from, to, std::move(piece),
                                      totalMs, 0, distance, msPerCell});
}

// ─────────────────────────────────────────────
// tick — קידום זמן לכל התנועות, איסוף מסתיימות
// ─────────────────────────────────────────────
std::vector<Motion> RealTimeArbiter::tick(int ms) {
    std::vector<Motion> completed;

    // קידום זמן לכל התנועות
    for (auto& motion : m_activeMotions) {
        motion.elapsedMs += ms;
        if (motion.elapsedMs >= motion.totalMs) {
            motion.elapsedMs = motion.totalMs; // clamp
        }
    }

    // איסוף תנועות שהסתיימו (שמירת סדר מקורי)
    // שימוש ב-stable_partition: כל התנועות שהסתיימו עוברות להתחלה
    auto firstActive = std::stable_partition(
        m_activeMotions.begin(),
        m_activeMotions.end(),
        [](const Motion& m) { return m.elapsedMs >= m.totalMs; }
    );

    // העברת התנועות שהושלמו לווקטור התוצאה
    completed.assign(
        std::make_move_iterator(m_activeMotions.begin()),
        std::make_move_iterator(firstActive)
    );

    // מחיקת התנועות שהושלמו מהווקטור הפעיל
    m_activeMotions.erase(m_activeMotions.begin(), firstActive);

    return completed;
}
