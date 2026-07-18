#pragma once
#include "Board.h"
#include "GameConfig.h"
#include "Position.h"
#include "PieceColor.h"
#include "RuleEngine.h"
#include "RealTimeArbiter.h"
#include "PieceStateConfig.h"
#include <optional>
#include <string>
#include <chrono>
#include <unordered_map>

/// תוצאת פעולת move
struct MoveResult final {
    bool success       = false;
    std::string message;       // סיבת כישלון ("" = הצלחה)
    bool capture       = false; // האם המהלך כלל הכאה
    bool gameOver      = false; // המשחק הסתיים אחרי המהלך
    bool motionStarted = false; // true = Arbiter התחיל תנועה (הלוח עוד לא עודכן)
};

/// תמונת מצב של המשחק — למנגנון שחזור/undo
struct GameSnapshot final {
    Board board;
    std::optional<Position> selectedPos;
    std::optional<Motion>   arbiterMotion;
    std::chrono::milliseconds gameClock{0};
    PieceColor currentTurn = PieceColor::White;
};

/// מצבי המשחק הראשיים
enum class GameState {
    Waiting,   // טרם התחיל — כלים מוצגים, ממתינים לפקודת התחלה
    Playing,   // משחק פעיל
    GameOver   // הסתיים
};

/// Timer for state transitions (e.g. long_rest → idle after N ms)
struct PieceStateTimer final {
    PieceState targetState;
    int        remainingMs;
};

/// ניהול מצב המשחק + תיאום שירות-אפליקציה.
/// SRP: ניהול בחירה, ניהול תורות, תנאי סיום, snapshots, האצלת אימות.
class GameEngine final {
public:
    GameEngine(Board board, GameConfig config);

    // ─── גישה לקריאה ───
    [[nodiscard]] const Board&      board()      const noexcept { return m_board; }
    [[nodiscard]] Board&            board()            noexcept { return m_board; }
    [[nodiscard]] const GameConfig& config()     const noexcept { return m_config; }
    [[nodiscard]] std::chrono::milliseconds game_clock() const noexcept { return m_gameClock; }
    [[nodiscard]] std::optional<Position> selected()  const noexcept { return m_selectedPos; }
    [[nodiscard]] PieceColor        current_turn()    const noexcept { return m_currentTurn; }
    [[nodiscard]] GameState         state()           const noexcept { return m_state; }
    [[nodiscard]] bool              is_busy()         const noexcept { return m_busy; }

    // ─── פעולות משחק ───
    void start_game() noexcept;
    bool select(Position pos);
    void deselect() noexcept { m_selectedPos.reset(); }

    /// אימות מהלך — שלב 1: בדיקת חוקיות בלבד (const), לא משנה מצב.
    /// @return MoveValidation — is_valid + reason.
    [[nodiscard]] MoveValidation validate_move(Position from, Position to) const;

    /// ביצוע מהלך לוגי — שלב 2: נקרא רק בהגעה.
    /// SRP: משנה את הלוח + בודק game-over.
    /// @pre המהלך כבר אומת כ-valid.
    MoveResult commit_move(Position from, Position to);

    /// עוטף את validate_move + commit_move עם לוגיקת בחירה.
    /// ללא Arbiter: מבצע מיידית (לשימוש ב-legacy / tests).
    /// עם Arbiter: מחזיר motionStarted=true, Controller מפעיל startMotion.
    MoveResult move_selected_to(Position target);

    // ─── PieceState Management (NEW) ───

    /// מתחיל תנועה: קובע PieceState.move + busy=true.
    /// טוען config ל-move, מחשב msPerCell מ-speed_m_per_sec.
    /// @return msPerCell לשימוש ב-RealTimeArbiter, או 0 בכשלון.
    [[nodiscard]] int start_piece_motion(Position from, Position to);

    /// commit_move + עדכון PieceState ל-next_state (long_rest/short_rest) + timer.
    void commit_move_with_state(Position from, Position to);

    /// קידום state timers (long_rest/short_rest → idle).
    void tick_state_machines(int deltaMs);

    /// גישה ל-PieceStateConfig (עם מטמון פנימי).
    [[nodiscard]] const PieceStateConfig& get_piece_state_config(
        const std::string& code, const std::string& stateName);

    /// סימון busy — נקבע ע"י Controller כשמתחילה תנועה.
    void set_busy(bool busy) noexcept { m_busy = busy; }

    // ─── סיום משחק ───
    [[nodiscard]] bool is_game_over() const noexcept { return m_state == GameState::GameOver; }
    [[nodiscard]] std::optional<PieceColor> winner() const noexcept { return m_winner; }

    // ─── Snapshots ───
    [[nodiscard]] GameSnapshot snapshot() const;
    void restore(const GameSnapshot& snap);

private:
    int ms_per_cell_from_speed(double speed_m_per_sec) const;
    PieceStateConfig load_and_cache_config(const std::string& code,
                                            const std::string& stateName);

    Board       m_board;
    GameConfig  m_config;
    std::optional<Position> m_selectedPos;
    std::chrono::milliseconds m_gameClock{0};
    PieceColor  m_currentTurn = PieceColor::White;
    std::optional<PieceColor> m_winner;
    GameState   m_state = GameState::Waiting;
    bool        m_busy  = false;  // true = יש תנועה פעילה, אין מהלכים חדשים

    // NEW: piece state config cache + state timers
    std::unordered_map<std::string, PieceStateConfig> m_pieceStateConfigCache;
    std::unordered_map<Position, PieceStateTimer> m_stateTimers;
};
