#pragma once
#include "Board.h"
#include "GameConfig.h"
#include "GameSnapshot.h"
#include "MoveExecutor.h"
#include "PieceStateMachine.h"
#include "Position.h"
#include "PieceColor.h"
#include "RuleEngine.h"
#include <optional>
#include <string>
#include <chrono>

/// תוצאת פעולת move
struct MoveResult final {
    bool success = false;
    std::string message;       // סיבת כישלון ("" = הצלחה)
    bool capture = false;      // האם המהלך כלל הכאה
    bool gameOver = false;     // המשחק הסתיים אחרי המהלך
    bool motionStarted = false; // true = Arbiter התחיל תנועה (הלוח עוד לא עודכן)
};

/// מצבי המשחק הראשיים
enum class GameState {
    Waiting,   // טרם התחיל — כלים מוצגים, ממתינים לפקודת התחלה
    Playing,   // משחק פעיל
    GameOver   // הסתיים
};

/// ניהול מצב המשחק — Application Service / Orchestrator.
///
/// SRP: GameEngine מתאם בין רכיבי המערכת:
///   • RuleEngine — אימות חוקיות מהלכים
///   • MoveExecutor — ביצוע מהלכים על הלוח
///   • PieceStateMachine — ניהול PieceState + timers + config
///   • RealTimeArbiter — ניהול תנועות בזמן (מתקבל כפרמטר)
///
/// GameEngine אינו מזיז כלים, אינו מצייר. הוא מתזמר.
/// GameEngine מחזיק GameState, GameSnapshot.
class GameEngine final {
public:
    GameEngine(Board board, GameConfig config,
               std::string piecesRootDir);

    // ─── ניהול משחק ───
    void start_game() noexcept;

    [[nodiscard]] const Board& board() const noexcept { return m_board; }
    [[nodiscard]] GameState state() const noexcept { return m_state; }
    [[nodiscard]] PieceColor current_turn() const noexcept { return m_currentTurn; }

    // ─── Selection ───
    [[nodiscard]] bool select(Position pos);
    void deselect() noexcept { m_selectedPos.reset(); }
    [[nodiscard]] const std::optional<Position>& selected() const noexcept { return m_selectedPos; }

    // ─── Validation ───
    /// האצלה ל-RuleEngine
    [[nodiscard]] MoveValidation validate_move(Position from, Position to) const;

    /// ביצוע מהלך לוגי — האצלה ל-MoveExecutor.
    /// @pre המהלך כבר אומת כ-valid.
    MoveResult commit_move(Position from, Position to);

    /// עוטף validate_move + MoveExecutor.execute + לוגיקת בחירה.
    /// לשימוש ב-legacy / tests (ללא Arbiter).
    MoveResult move_selected_to(Position target);

    // ─── Motion orchestration ───

    /// התחלת תנועה: האצלה ל-PieceStateMachine.
    /// @return msPerCell לשימוש ב-RealTimeArbiter, או 0 בכשלון.
    [[nodiscard]] int startMotion(Position from, Position to);

    /// התחלת קפיצה במקום (קליק כפול על כלי).
    /// @return true אם הקפיצה התחילה
    [[nodiscard]] bool startJump(Position pos);

    /// קידום זמן מלא: state timers + arbiter + השלמת מהלכים.
    /// @return true אם לפחות מהלך אחד הושלם ב-tick הזה.
    [[nodiscard]] bool tick(int deltaMs, RealTimeArbiter& arbiter);

    // ─── סיום משחק ───
    [[nodiscard]] bool is_game_over() const noexcept { return m_state == GameState::GameOver; }
    [[nodiscard]] std::optional<PieceColor> winner() const noexcept { return m_winner; }

    // ─── Snapshots ───
    [[nodiscard]] GameSnapshot snapshot() const;
    void restore(const GameSnapshot& snap);

private:
    Board              m_board;
    GameConfig         m_config;
    PieceStateMachine  m_stateMachine;
    MoveExecutor       m_moveExecutor;
    std::optional<Position> m_selectedPos;
    std::chrono::milliseconds m_gameClock{0};
    PieceColor              m_currentTurn = PieceColor::White;
    std::optional<PieceColor> m_winner;
    GameState               m_state = GameState::Waiting;
    std::optional<Position> m_jumpingPos;   // מעקב אחרי קפיצה פעילה
};
