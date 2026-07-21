#pragma once

#include "../Model/Piece.h"

/// טיפול בקידום רגלי — Pure logic, no side effects.
/// בודק האם כלי הוא רגלי שהגיע לשורה האחרונה ומקדם למלכה.
///
/// @param piece  הכלי לבדיקה (אחרי שהוצב במיקום החדש)
/// @param rows   מספר השורות בלוח (לקביעת שורת הקידום)
/// @return       Piece — המקור אם לא היה קידום, או Piece חדש (queen) אם התקיים קידום
inline Piece try_promote_pawn(Piece piece, int rows) {
    if (piece.type_id() != "pawn") return piece;

    int promoRow = (piece.get_color() == PieceColor::White) ? 0 : rows - 1;
    if (piece.get_pos().row != promoRow) return piece;

    return Piece(piece.get_color(), "queen", piece.get_pos());
}
