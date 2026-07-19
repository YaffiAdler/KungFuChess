#pragma once
#include "Piece.h"
#include "PieceStateConfig.h"
#include "Position.h"
#include "GameConfig.h"
#include <string>
#include <unordered_map>
#include <vector>

/// תוצאת tick של state machine — איזה כלי צריך לעדכן state.
struct StateTimerUpdate final {
    Position   pos;
    PieceState newState;
};

/// Timer for state transitions (e.g. long_rest → idle after N ms)
struct PieceStateTimer final {
    PieceState targetState;
    int        remainingMs;
};

/// ניהול מחזור החיים של PieceState + טעינת config + timers.
///
/// SRP: PieceStateMachine אחראי על:
/// • טעינה lazy + caching של PieceStateConfig
/// • חישוב msPerCell מ-speed_m_per_sec
/// • startMotion: PieceState::idle → PieceState::move
/// • completeMotion: move → next_state (long_rest/short_rest) + timer
/// • tick: קידום timers → StateTimerUpdate list
///
/// PieceStateMachine אינו נוגע בלוח (Board), אינו יוצר Motion,
/// אינו בודק חוקיות שחמט.
class PieceStateMachine final {
public:
    PieceStateMachine(const GameConfig& config, std::string rootDir);

    /// התחלת תנועה: קובע PieceState::move, מחשב msPerCell.
    /// @return msPerCell (0 בכשלון)
    [[nodiscard]] int startMotion(Piece& piece, Position from, Position to);

    /// התחלת קפיצה במקום: קובע PieceState::jump + timer ל-next_state.
    /// @return true אם הקפיצה התחילה
    [[nodiscard]] bool startJump(Piece& piece);

    /// השלמת תנועה: קובע next_state + timer אם צריך.
    /// @param piece           הכלי (אחרי commit_move, במיקום היעד)
    /// @param code            קוד הכלי (piece.get_code())
    /// @param completedState  ה-state שהסתיים (בד"כ "move")
    void completeMotion(Piece& piece, const std::string& code,
                        const std::string& completedState);

    /// קידום timers — מחזיר רשימת עדכונים ל-GameEngine.
    [[nodiscard]] std::vector<StateTimerUpdate> tick(int deltaMs);

    /// איפוס timers (עבור snapshot/restore)
    void reset() noexcept { m_stateTimers.clear(); }

private:
    /// גישה לפרטי config של code + state (עם lazy-load + cache)
    [[nodiscard]] const PieceStateConfig& getConfig(
        const std::string& code, const std::string& stateName);

    int ms_per_cell_from_speed(double speed_m_per_sec) const;

    const GameConfig& m_config;
    std::string               m_rootDir;
    std::unordered_map<std::string, PieceStateConfig> m_configCache;
    std::unordered_map<Position, PieceStateTimer>     m_stateTimers;
};
