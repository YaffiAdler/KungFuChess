#include "RealTimeArbiter.h"
#include <algorithm>  // std::max

// ─────────────────────────────────────────────
//  startMotion — התחלת תנועה
// ─────────────────────────────────────────────
bool RealTimeArbiter::startMotion(Position from, Position to,
                                   Piece piece, const Board& /*board*/) {
    // כבר יש תנועה פעילה — דחה
    if (m_activeMotion.has_value()) {
        return false;
    }

    // חישוב זמן תנועה: מרחק צ'בישב × 1000ms
    int distance = chebyshevDistance(from, to);
    int totalMs = distance * 1000;  // CELL_SIZE / PIECE_SPEED * 1000 = 1000ms/cell

    // בדיקת אכילה — האם היעד תפוס על ידי כלי אויב?
    // נבדוק ב-tick() כדי לקבל kingEaten, אבל נרשום wasCapture מראש.
    // בפועל, wasCapture ייקבע ב-tick לפי מצב הלוח בזמן ההגעה.
    // כאן רק נרשום את המידע הראשוני.

    m_activeMotion = Motion{
        from,
        to,
        std::move(piece),
        totalMs,
        0  // elapsedMs מתחיל מ-0
    };

    return true;
}

// ─────────────────────────────────────────────
//  tick — קידום זמן
// ─────────────────────────────────────────────
ArbiterTickResult RealTimeArbiter::tick(int ms) {
    ArbiterTickResult result;

    if (!m_activeMotion.has_value()) {
        return result;  // אין תנועה פעילה — כלום לא קרה
    }

    m_activeMotion->elapsedMs += ms;

    // בדיקה: האם הזמן הכולל עבר?
    if (m_activeMotion->elapsedMs >= m_activeMotion->totalMs) {
        result.completed = true;

        // שמירת מידע התנועה לפני ניקוי
        // (ה-wasCapture וה-kingEaten ייקבעו ע"י GameEngine לפי מצב הלוח)
        // RealTimeArbiter לא נוגע בלוח — GameEngine יבדוק בעצמו
    }

    return result;
}

// ─────────────────────────────────────────────
//  cancelMotion — ביטול תנועה
// ─────────────────────────────────────────────
Piece RealTimeArbiter::cancelMotion() {
    // החזר את הכלי — GameEngine ידאג להחזיר אותו ללוח אם צריך
    Piece p = std::move(m_activeMotion->piece);
    m_activeMotion.reset();
    return p;
}
