#pragma once
#include "../Model/Position.h"
#include "../Model/Piece.h"
#include "../Model/PieceColor.h"
#include "../Model/Board.h"
#include <optional>
#include <vector>
#include <cstdlib> // abs

/// אובייקט תנועה פעילה — מייצג כלי בתנועה בין שני תאים.
/// בבעלות RealTimeArbiter, לא הלוח.
struct Motion final {
    Position from;
    Position to;
    Piece piece;        // העתק של הכלי הנע (כולל צבע, סוג, מיקום)
    int totalMs;        // זמן כולל לתנועה (distance × msPerCell)
    int elapsedMs;      // זמן שחלף עד כה
    int distance = 0;   // מרחק צ'בישב (מספר תאים)
    int msPerCellUsed = 0; // msPerCell שחושב מ-speed_m_per_sec
};

/// תוצאת קידום זמן — מה קרה ב-tick האחרון עבור תנועה בודדת.
struct ArbiterTickResult {
    bool completed = false;   // תנועה הסתיימה (הגעה ליעד)
};

/// ארביטר תנועה בזמן אמת — תומך בריבוי תנועות בו-זמנית.
///
/// SRP: תפקיד יחיד — ניהול תנועות פעילות לאורך זמן.
/// RealTimeArbiter אינו מעדכן את הלוח, אינו בודק חוקיות שחמט,
/// אינו מטפל בקליקים או ברינדור.
///
/// כל תנועה מתחילה ב-startMotion() (שתמיד מצליח — אין הגבלה).
/// כשה-tick מגלה שתנועה הסתיימה, היא מוחזרת בתוצאה ומוסרת מהאוסף.
class RealTimeArbiter final {
public:
    RealTimeArbiter() = default;

    /// התחלת תנועה חדשה.
    /// @param from מיקום מקור
    /// @param to   מיקום יעד
    /// @param piece העתק של הכלי הנע
    /// @param msPerCell זמן לאלפית שנייה לתא (מ-GameConfig / PieceStateMachine)
    /// (אין הגבלה על מספר התנועות הפעילות)
    void startMotion(Position from, Position to,
                     Piece piece, int msPerCell);

    /// התחלת תנועה עם מרחק חיצוני (גרסה מותאמת).
    /// @param distance מרחק צ'בישב שחושב מראש
    void startMotionWithDistance(Position from, Position to,
                                  Piece piece,
                                  int distance, int msPerCell);

    /// קידום זמן — מקדם את כל התנועות הפעילות.
    /// @param ms אלפיות שנייה שחלפו
    /// @return וקטור של תנועות שהסתיימו ב-tick הזה (לפי סדר ההשלמה)
    [[nodiscard]] std::vector<Motion> tick(int ms);

    /// בדיקה: האם יש תנועות פעילות?
    [[nodiscard]] bool hasActiveMotion() const noexcept {
        return !m_activeMotions.empty();
    }

    /// גישה ישירה לווקטור התנועות (עבור Renderer — אינטרפולציה)
    [[nodiscard]] const std::vector<Motion>& motions() const noexcept {
        return m_activeMotions;
    }

    /// שחזור תנועות (מ-snapshot)
    void setMotions(std::vector<Motion> motions) noexcept {
        m_activeMotions = std::move(motions);
    }

private:
    std::vector<Motion> m_activeMotions;

    /// חישוב מרחק צ'בישב: max(|dr|, |dc|)
    static int chebyshevDistance(Position a, Position b) noexcept {
        int dr = std::abs(a.row - b.row);
        int dc = std::abs(a.col - b.col);
        return dr > dc ? dr : dc;
    }
};
