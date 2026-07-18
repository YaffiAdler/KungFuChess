#pragma once
#include "Position.h"
#include "PieceColor.h"
#include <string>
#include <vector>
#include <cctype>

enum class PieceState
{
    idle,
    jump,
    long_rest,
    move,
    short_rest
};

/// Convert PieceState to string (e.g. PieceState::move → "move")
inline std::string state_to_string(PieceState state) {
    switch (state) {
        case PieceState::idle:       return "idle";
        case PieceState::move:       return "move";
        case PieceState::jump:       return "jump";
        case PieceState::long_rest:  return "long_rest";
        case PieceState::short_rest: return "short_rest";
        default: return "idle";
    }
}

/// Convert string to PieceState (e.g. "move" → PieceState::move)
inline PieceState state_from_string(const std::string& name) {
    if (name == "idle")       return PieceState::idle;
    if (name == "move")       return PieceState::move;
    if (name == "jump")       return PieceState::jump;
    if (name == "long_rest")  return PieceState::long_rest;
    if (name == "short_rest") return PieceState::short_rest;
    return PieceState::idle;
}

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

    /// קוד כלי: "BB", "KW", "RB" וכו'
    [[nodiscard]] std::string get_code() const;

    /// סימבול הכלי מתוך ה-Registry ('K', 'Q', ...)
    [[nodiscard]] char get_symbol() const;

    void set_pos(Position newPos) noexcept { m_pos = newPos; }
    void mark_moved()            noexcept { m_hasMoved = true; }

    /// ייצוג טקסטואלי: "wK", "bP" וכו'
    [[nodiscard]] std::string to_token() const;

    /// המהלכים האפשריים לפי MovementRules שב-Registry (גנרי לחלוטין)
    [[nodiscard]] std::vector<Position>
    get_raw_moves(int numRows, int numCols) const;

    /// @brief מחזיר את המצב הנוכחי של כלי
    /// @return  מצב הכלי
    PieceState get_state() const;
    void set_state(PieceState state);

private:
    PieceColor   m_color;
    std::string  m_typeId;    // "king", "queen", ... — DATA, לא enum
    Position     m_pos;
    bool         m_hasMoved = false;
    PieceState   p_state =PieceState::idle;
};
