#pragma once
#include "Board.h"
#include "Position.h"
#include "PieceColor.h"
#include <string>
#include <optional>

/// תוצאת אימות מהלך — קריאה-בלבד, ללא שינוי מצב.
struct MoveValidation final {
    bool        is_valid = false;
    std::string reason;     // "ok" = חוקי. אחרת: סיבה קריאה-למכונה.
};

/// אימות חוקיות מהלך + בדיקת סיום משחק. קריאה-בלבד — לא משנה Board.
///
/// SRP: תפקיד יחיד — להחליט האם מהלך (from→to) חוקי לפי מצב הלוח.
/// RuleEngine לא מזיז כלים, לא מסיר אכילות, לא מנהל תורות,
/// לא בודק שח/נעילות/הצרחה, ולא קובע סיום משחק.
class RuleEngine final {
public:
    explicit RuleEngine(const Board& board) noexcept : m_board(&board) {}

    /// אימות מהלך. בודק: גבולות לוח, תא מקור לא ריק,
    /// יעד לא תפוס על ידי כלי ידידותי, וכללי תנועה.
    [[nodiscard]] MoveValidation validate_move(Position from, Position to) const noexcept;

    /// בדיקת סיום משחק: בודק אם צבע כלשהו איבד את המלך.
    /// מחזיר את הצבע המנצח, או std::nullopt אם המשחק ממשיך.
    /// קריאה-בלבד — לא משנה Board, לא קובע GameState.
    /// GameEngine קורא לפונקציה ומעדכן את m_state / m_winner בעצמו.
    [[nodiscard]] static std::optional<PieceColor>
    check_game_over(const Board& board) noexcept;

private:
    const Board* m_board;
};
