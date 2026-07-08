#pragma once
#include "Board.h"
#include "GameConfig.h"
#include "Position.h"
#include <optional>
#include <string>
#include <chrono>

/// תוצאת עיבוד פקודה
struct CommandResult final {
    bool        success = true;
    std::string message;            // פלט להדפסה (ריק = אין)
    bool        exitRequested = false;  // שמור לעתיד
};

/// ניהול מצב המשחק.
/// SRP: עיבוד פקודות, ניהול בחירה, שעון משחק.
class Game final {
public:
    Game(Board board, GameConfig config);

    // ─── גישה לקריאה ───
    [[nodiscard]] const Board&    board()              const noexcept { return m_board; }
    [[nodiscard]] const GameConfig& config()           const noexcept { return m_config; }
    [[nodiscard]] std::chrono::milliseconds game_clock() const noexcept { return m_gameClock; }
    [[nodiscard]] std::optional<Position> selected()   const noexcept { return m_selectedPos; }

    /// מעבד שורת פקודה ומחזיר תוצאה
    [[nodiscard]] CommandResult execute_command(const std::string& cmdLine);

    /// הדפסת מצב הלוח הנוכחי (כולל סימון [XY] לכלי נבחר)
    [[nodiscard]] std::string board_string() const;

private:
    // ─── פקודות ───
    CommandResult handle_click(int x, int y);
    CommandResult handle_wait(int ms);
    CommandResult handle_print_board();

    /// המרת קואורדינטות פיקסלים לתא לוח. nullopt = מחוץ ללוח.
    [[nodiscard]] std::optional<Position> pixel_to_cell(int x, int y) const noexcept;

    // ─── נתונים ───
    Board                           m_board;
    GameConfig                      m_config;
    std::optional<Position>         m_selectedPos;
    std::chrono::milliseconds       m_gameClock{0};
};
