#pragma once
#include "Board.h"
#include "GameConfig.h"
#include "Position.h"
#include "PieceColor.h"
#include <optional>
#include <string>
#include <chrono>

/// תוצאת פעולת move
struct MoveResult final {
    bool        success = false;
    std::string message;            // סיבת כישלון (ריק = הצלחה)
    bool        capture = false;    // האם המהלך כלל הכאה
    bool        gameOver = false;   // האם המשחק הסתיים אחרי המהלך
};

/// תמונת מצב של המשחק — למנגנון שחזור/undo
struct GameSnapshot final {
    Board                          board;
    std::optional<Position>        selectedPos;
    std::chrono::milliseconds      gameClock{0};
    PieceColor                     currentTurn = PieceColor::White;
};

/// ניהול מצב המשחק — תיאום שירות-אפליקציה.
/// SRP: ניהול בחירה, ניהול תורות, תנאי סיום, snapshots, האצלת אימות.
class GameEngine final {
public:
    GameEngine(Board board, GameConfig config);

    // ─── גישה לקריאה ───
    [[nodiscard]] const Board&    board()              const noexcept { return m_board; }
    [[nodiscard]] const GameConfig& config()           const noexcept { return m_config; }
    [[nodiscard]] std::chrono::milliseconds game_clock() const noexcept { return m_gameClock; }
    [[nodiscard]] std::optional<Position> selected()   const noexcept { return m_selectedPos; }
    [[nodiscard]] PieceColor      current_turn()       const noexcept { return m_currentTurn; }

    // ─── פעולות משחק ───
    /// בחירת כלי במיקום נתון. מחזיר true אם יש כלי לבחור.
    [[nodiscard]] bool select(Position pos);

    /// ביצוע מהלך (from→to) עם אימות דרך RuleEngine.
    /// SRP: תיאום — בודק game_over, מאציל אימות ל-RuleEngine, מבצע מהלך.
    [[nodiscard]] MoveResult request_move(Position from, Position to);

    /// הזזת הכלי הנבחר למיקום היעד. קורא ל-request_move.
    [[nodiscard]] MoveResult move_selected_to(Position target);

    /// ביטול בחירה
    void deselect() noexcept { m_selectedPos.reset(); }

    // ─── שעון ───
    void advance_clock(std::chrono::milliseconds ms) noexcept { m_gameClock += ms; }

    // ─── סיום משחק ───
    [[nodiscard]] bool is_game_over() const noexcept { return m_gameOver; }
    [[nodiscard]] std::optional<PieceColor> winner() const noexcept { return m_winner; }

    // ─── Snapshots ───
    [[nodiscard]] GameSnapshot snapshot() const;
    void restore(const GameSnapshot& snap);

private:

    Board                           m_board;
    GameConfig                      m_config;
    std::optional<Position>         m_selectedPos;
    std::chrono::milliseconds       m_gameClock{0};
    PieceColor                      m_currentTurn = PieceColor::White;
    bool                            m_gameOver = false;
    std::optional<PieceColor>       m_winner;
};
