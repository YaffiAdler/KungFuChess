#pragma once
#include "Board.h"
#include "Position.h"
#include "PieceColor.h"
#include <optional>

/// תוצאת ביצוע מהלך — מה קרה בלוח.
/// MoveExecutor אינו מעדכן GameState — GameEngine עושה זאת
/// על סמך ה-gameOver / winner שבדיווח.
struct MoveExecResult final {
    bool success = false;
    bool capture = false;
    bool gameOver = false;
    std::optional<PieceColor> winner;
};

/// מבצע מהלך בלוח — SRP: שינוי הלוח בלבד.
///
/// MoveExecutor לא בודק חוקיות (RuleEngine עשה זאת).
/// MoveExecutor לא מעדכן GameState / PieceState / Config.
/// הוא רק מזיז כלים, מכה, מקדם, ובודק סיום (מלך חסר).
class MoveExecutor final {
public:
    explicit MoveExecutor(Board& board) noexcept : m_board(&board) {}

    /// ביצוע מהלך שאומת מראש.
    /// @pre המהלך עבר RuleEngine::validate_move
    /// @param from מיקום מקור
    /// @param to   מיקום יעד
    /// @return MoveExecResult — success, capture, gameOver, winner
    [[nodiscard]] MoveExecResult execute(Position from, Position to);

private:
    Board* m_board;
};
