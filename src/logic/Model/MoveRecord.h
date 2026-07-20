#pragma once
#include "Position.h"
#include "Piece.h"
#include "PieceColor.h"
#include <optional>
#include <string>

/// תיעוד של מהלך שהושלם — Data טהור, ללא לוגיקה.
struct MoveRecord final {
    Position from;                           // מיקום מקור
    Position to;                             // מיקום יעד
    Piece   piece;                           // העתק של הכלי שביצע את המהלך
    bool    capture = false;                 // האם המהלך כלל הכאה
    std::optional<Piece> capturedPiece;      // הכלי שהוכה (אם היה כזה)
    PieceColor player;                       // צבע הכלי שביצע את המהלך
};

/// המרת MoveRecord לתווי שחמט אלגברי סטנדרטי.
/// @param record המהלך להמרה
/// @param numRows מספר השורות בלוח (לחישוב דרגה)
/// @return מחרוזת בתווי אלגברי, למשל "Qe8", "exd4", "Nf3", "gxf6"
[[nodiscard]] std::string to_algebraic(const MoveRecord& record, int numRows = 8);
