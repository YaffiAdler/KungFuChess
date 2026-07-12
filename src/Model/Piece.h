#pragma once
#include "Position.h"
#include "PieceColor.h"
#include <string>
#include <vector>

/// כלי שחמט קונקרטי.
/// סוג הכלי נקבע לפי מזהה מחרוזת (למשל "king") —
/// לא enum! הכל מוגדר ב-PieceTypeRegistry כנתונים.
class Piece final {
public:
    Piece(PieceColor color, std::string typeId, Position pos);

    // ─── גישה בסיסית ───
    [[nodiscard]] PieceColor       get_color()  const noexcept { return m_color; }
    [[nodiscard]] const std::string& type_id()  const noexcept { return m_typeId; }
    [[nodiscard]] Position         get_pos()    const noexcept { return m_pos; }
    // [[nodiscard]] bool             has_moved()  const noexcept { return m_hasMoved; }

    /// סימבול הכלי מתוך ה-Registry ('K', 'Q', ...)
    [[nodiscard]] char get_symbol() const;

    void set_pos(Position newPos) noexcept { m_pos = newPos; }
    void mark_moved()            noexcept { m_hasMoved = true; }

    /// ייצוג טקסטואלי: "wK", "bP" וכו'
    [[nodiscard]] std::string to_token() const;

    /// המהלכים האפשריים לפי MovementRules שב-Registry (גנרי לחלוטין)
    [[nodiscard]] std::vector<Position>
    get_raw_moves(int numRows, int numCols) const;

private:
    PieceColor   m_color;
    std::string  m_typeId;    // "king", "queen", ... — DATA, לא enum
    Position     m_pos;
    bool         m_hasMoved = false;
};
