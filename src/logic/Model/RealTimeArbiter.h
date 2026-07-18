#pragma once
#include "Position.h"
#include "Piece.h"
#include "PieceColor.h"
#include "Board.h"
#include <optional>
#include <cstdlib> // abs

/// אובייקט תנועה פעילה — מייצג כלי בתנועה בין שני תאים.
/// בבעלות RealTimeArbiter, לא הלוח.
struct Motion final {
    Position from;
    Position to;
    Piece   piece;      // העתק של הכלי הנע (כולל צבע, סוג, מיקום)
    int     totalMs;    // זמן כולל לתנועה (distance × msPerCell)
    int     elapsedMs;  // זמן שחלף עד כה
    int     distance   = 0;    // מרחק צ'בישב (מספר תאים)
    int     msPerCellUsed = 0; // msPerCell שחושב מ-speed_m_per_sec
};

/// תוצאת קידום זמן — מה קרה ב-tick האחרון.
struct ArbiterTickResult {
    bool completed  = false;  // תנועה הסתיימה (הגעה ליעד)
    bool wasCapture = false;  // היה כלי אויב ביעד
};

/// ארביטר תנועה בזמן אמת.
///
/// SRP: תפקיד יחיד — ניהול תנועות פעילות לאורך זמן.
/// RealTimeArbiter אינו מעדכן את הלוח, אינו בודק חוקיות שחמט,
/// אינו מטפל בקליקים או ברינדור.
///
/// הוא מקבל פקודת מהלך (שכבר אומתה כ-valid) ומנהל את משך הזמן שלה.
/// כשה-tick מסמן completed, ה-Controller מתזמן commit_move בלוח.
class RealTimeArbiter final {
public:
    RealTimeArbiter() = default;

    /// התחלת תנועה חדשה.
    /// @param from  מיקום מקור
    /// @param to    מיקום יעד
    /// @param piece העתק הכלי הנע
    /// @param msPerCell זמן לצעד-תא (מ-GameConfig)
    /// @return true אם התנועה התחילה, false אם כבר יש תנועה פעילה
    bool startMotion(Position from, Position to, Piece piece, int msPerCell);

    /// התחלת תנועה עם distance ו-msPerCell ידניים (מתוך speed_m_per_sec).
    /// @param from     מיקום מקור
    /// @param to       מיקום יעד
    /// @param piece    העתק הכלי הנע
    /// @param distance מרחק צ'בישב (לחישוב totalMs)
    /// @param msPerCell זמן לצעד-תא (מחושב מ-speed_m_per_sec)
    /// @return true אם התנועה התחילה
    bool startMotionWithDistance(Position from, Position to, Piece piece,
                                  int distance, int msPerCell);

    /// קידום זמן — צובר elapsedMs.
    /// @param ms  אלפיות שנייה שחלפו
    /// @return תוצעת tick — completed, wasCapture
    ArbiterTickResult tick(int ms);

    /// בדיקה: האם יש תנועה פעילה?
    [[nodiscard]] bool hasActiveMotion() const noexcept {
        return m_activeMotion.has_value();
    }

    /// גישה ישירה ל-optional motion (עבור Renderer — אינטרפולציה)
    [[nodiscard]] const std::optional<Motion>& motion() const noexcept {
        return m_activeMotion;
    }

    /// שחזור תנועה (מ-snapshot)
    void setMotion(std::optional<Motion> motion) noexcept {
        m_activeMotion = std::move(motion);
    }

private:
    std::optional<Motion> m_activeMotion;

    /// חישוב מרחק צ'בישב: max(|dr|, |dc|)
    static int chebyshevDistance(Position a, Position b) noexcept {
        int dr = std::abs(a.row - b.row);
        int dc = std::abs(a.col - b.col);
        return dr > dc ? dr : dc;
    }
};
