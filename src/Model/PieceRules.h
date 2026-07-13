#pragma once
#include "Board.h"
#include "Position.h"

struct MovementRule;  // forward-declare — PieceRules uses it as output

/// כללי תנועה ספציפיים לפי סוג מהלך (Slide/Step/Jump).
/// SRP: לוגיקת חסימות והכאה — בתיווך בין MoveGenerator (גאומטריה) ל-RuleEngine (אימות).
///
/// PieceRules בודק: האם המהלך חסום על ידי כלים אחרים?
/// - Slide: לא יכול לעבור דרך כלים. התא האחרון (to) יכול להיות אויב (הכאה).
/// - Step: צעד בודד — לא יכולים להיות חוסמים בדרך.
///          צעד כפול (רגלי) — התא האמצעי חייב להיות ריק.
/// - Jump: קפיצה (Knight) — לא נחסמת, מדלגת מעל כלים.
///
/// @param matched_rule (אופציונלי) — אם המהלך חוקי, יצביע על ה-MovementRule שהתאים.
///         משמש את RuleEngine לבדיקת canCapture.
class PieceRules final {
public:
    /// בדיקה מלאה: בהינתן כלי, לוח, from, to —
    /// האם יש חוסם בדרך שמונע את המהלך?
    ///
    /// @param board         הלוח
    /// @param piece         הכלי המזיז
    /// @param from          תא מקור
    /// @param to            תא יעד
    /// @param matched_rule  [out] אופציונלי — יצביע על MovementRule תואם
    /// @return true אם המהלך חוקי ברמת חסימות/הכאה.
    ///         false עם reason מתאים.
    [[nodiscard]] static bool
    is_path_clear(const Board& board, const Piece& piece,
                  Position from, Position to,
                  const MovementRule** matched_rule = nullptr) noexcept;

    /// החזרת סיבת הכישלון האחרונה (לשימוש RuleEngine)
    [[nodiscard]] static const char* last_block_reason() noexcept { return m_lastReason; }

private:
    /// בדיקת חסימה ל-Slide: צעד תא-תא, כל תא ביניים חייב להיות ריק.
    /// התא האחרון (to) יכול להיות ריק או אויב.
    [[nodiscard]] static bool slide_path_clear(const Board& board,
                                                Position from, Position to);

    /// בדיקת חסימה ל-Step: התא האמצעי (לצעד כפול) חייב להיות ריק.
    [[nodiscard]] static bool step_path_clear(const Board& board,
                                               Position from, Position to,
                                               int maxSteps);

    static const char* m_lastReason;
};
