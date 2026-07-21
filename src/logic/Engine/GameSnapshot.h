#pragma once
#include "../Model/Position.h"
#include "../Model/PieceColor.h"
#include "../Model/Piece.h"         // for PieceState enum only
#include "../Model/GameState.h"
#include <string>
#include <vector>
#include <optional>

/// מידע שטוח על כלי בודד — DTO, ללא מצביעים לאובייקטי Domain.
/// SnapshotBuilder הוא היחיד שמייצר PieceInfo מתוך Piece.
struct PieceInfo final {
    std::string typeId;       // "king", "queen", ...
    std::string code;         // "WK", "BB", ...
    PieceColor color;
    Position   pos;           // מיקום לוגי על הלוח
    PieceState state;         // idle, move, jump, long_rest, short_rest
};

/// מידע שטוח על כלי בתנועה — DTO, ללא תלות ב-Motion/Piece.
/// SnapshotBuilder מחשב progress מתוך Motion::elapsedMs / totalMs.
struct MotionInfo final {
    PieceInfo piece;
    Position  from;
    Position  to;
    double    progress = 0.0;  // 0.0 עד 1.0, מחושב מראש
};

/// תמונת מצב מלאה של המשחק — DTO בלבד.
///
/// מכיל רק מידע שטוח: אין מצביעים, אין אובייקטי Domain.
/// שכבת הגרפיקה עובדת אך ורק מול GameSnapshot.
struct GameSnapshot final {
    int boardRows = 0;
    int boardCols = 0;
    std::vector<PieceInfo> pieces;              // כל הכלים שעל הלוח
    std::vector<MotionInfo> motions;            // כל הכלים בתנועה (עם progress)
    std::optional<Position> selectedPos;        // תא נבחר
    std::optional<PieceColor> winner;           // מנצח (אם המשחק נגמר)
    GameState state = GameState::Waiting;       // מצב המשחק
    PieceColor currentTurn = PieceColor::White; // תור נוכחי
};
