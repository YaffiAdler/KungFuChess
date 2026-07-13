#pragma once
#include "Position.h"
#include "Piece.h"
#include "PieceColor.h"
#include "Board.h"
#include <optional>
#include <cstdlib>  // abs

/// אובייקט תנועה פעילה — מייצג כלי בתנועה בין שני תאים.
/// בבעלות RealTimeArbiter, לא הלוח.
struct Motion final {
    Position from;
    Position to;
    Piece   piece;      // העתק של הכלי הנע (כולל צבע, סוג, מיקום)
    int     totalMs;    // זמן כולל לתנועה (N תאים × 1000ms)
    int     elapsedMs;  // זמן שחלף עד כה
};

/// תוצאת קידום זמן — מה קרה ב-tick האחרון.
struct ArbiterTickResult final {
    bool                        completed  = false;  // תנועה הסתיימה (הגעה ליעד)
    bool                        wasCapture = false;  // היה כלי אויב ביעד
    std::optional<PieceColor>   kingEaten;           // מלך נאכל (game over)
};

/// ארביטר תנועה בזמן אמת.
///
/// SRP: תפקיד יחיד — ניהול תנועות פעילות לאורך זמן.
/// RealTimeArbiter אינו מעדכן את הלוח, אינו בודק חוקיות שחמט,
/// אינו מטפל בקליקים או ברינדור.
///
/// הוא מקבל פקודות מהלך שכבר אומתו, ומנהל תנועות פעילות ככל שהזמן מתקדם.
/// GameEngine משתמש בתוצאות tick() כדי לעדכן את הלוח.
class RealTimeArbiter final {
public:
    RealTimeArbiter() = default;

    /// התחלת תנועה חדשה. מחזיר false אם כבר יש תנועה פעילה.
    /// @param from תא מקור
    /// @param to   תא יעד
    /// @param piece העתק של הכלי הנע
    /// @param board הלוח (לקריאה בלבד — לבדיקת אכילה)
    [[nodiscard]] bool startMotion(Position from, Position to,
                                   Piece piece, const Board& board);

    /// קידום זמן מדומה.
    /// @param ms אלפיות שנייה לקידום
    /// @return תוצאת tick — האם התנועה הסתיימה, אכילה, מלך נאכל
    ArbiterTickResult tick(int ms);

    /// האם יש תנועה פעילה כרגע?
    [[nodiscard]] bool hasActiveMotion() const noexcept {
        return m_activeMotion.has_value();
    }

    /// גישה קריאה-בלבד לתנועה הפעילה (לתצוגה / אינטרפולציה).
    /// מחזיר nullptr אם אין תנועה פעילה.
    [[nodiscard]] const Motion* activeMotion() const noexcept {
        return m_activeMotion.has_value() ? &*m_activeMotion : nullptr;
    }

    /// ביטול תנועה פעילה — מחזיר את הכלי למצבו המקורי.
    /// @return הכלי שבוטלה תנועתו (להחזרה ללוח ב-Undo)
    Piece cancelMotion();

    /// גישה ישירה ל-optional motion (עבור snapshot/restore)
    [[nodiscard]] const std::optional<Motion>& motion() const noexcept {
        return m_activeMotion;
    }

    /// שחזור תנועה (מ-snapshot)
    void setMotion(std::optional<Motion> motion) noexcept {
        m_activeMotion = std::move(motion);
    }

private:
    std::optional<Motion> m_activeMotion;

    /// חישוב מרחק צ'בישב — max(|dr|, |dc|)
    static int chebyshevDistance(Position a, Position b) noexcept {
        int dr = std::abs(a.row - b.row);
        int dc = std::abs(a.col - b.col);
        return dr > dc ? dr : dc;
    }
};
