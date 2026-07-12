#pragma once
#include "Board.h"
#include "Position.h"
#include <string>

/// תוצאת אימות מהלך — קריאה-בלבד, ללא שינוי מצב.
struct MoveValidation final {
    bool        is_valid = false;
    std::string reason;     // "ok" = חוקי. אחרת: סיבה קריאה-למכונה.
};

/// אימות חוקיות מהלך. קריאה-בלבד — לא משנה Board.
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

private:
    const Board* m_board;
};
