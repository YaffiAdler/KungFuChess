#include "PieceStateMachine.h"
#include <cmath>

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
PieceStateMachine::PieceStateMachine(const GameConfig& config,
                                     std::string rootDir)
    : m_config(config)
    , m_rootDir(std::move(rootDir))
{}

// ─────────────────────────────────────────────
// startMotion — idle → move, returns msPerCell
// ─────────────────────────────────────────────
int PieceStateMachine::startMotion(Piece& piece, Position /*from*/, Position /*to*/) {
    std::string code = piece.get_code();

    const auto& cfg = getConfig(code, "move");

    piece.set_state(PieceState::move);

    return ms_per_cell_from_speed(cfg.speed_m_per_sec);
}

// ─────────────────────────────────────────────
// startJump — idle → jump + timer → (chain via tick)
// ─────────────────────────────────────────────
bool PieceStateMachine::startJump(Piece& piece) {
    std::string code = piece.get_code();

    const auto& jumpCfg = getConfig(code, "jump");

    piece.set_state(PieceState::jump);

    // Timer: jump → next_state (e.g. short_rest).
    // tick() chains short_rest → idle automatically.
    PieceState nextState = state_from_string(jumpCfg.next_state_when_finished);
    m_stateTimers[piece.get_pos()] = PieceStateTimer{
        nextState, m_config.jumpDurationMs};

    return true;
}

// ─────────────────────────────────────────────
// completeMotion — move → next_state + timer
// ─────────────────────────────────────────────
void PieceStateMachine::completeMotion(Piece& piece,
                                        const std::string& code,
                                        const std::string& completedState) {
    const auto& moveCfg = getConfig(code, completedState);
    std::string nextStateName = moveCfg.next_state_when_finished;
    PieceState nextState = state_from_string(nextStateName);

    piece.set_state(nextState);

    // טיימר ל-rest state → idle
    int durationMs = 0;
    if (nextState == PieceState::long_rest)
        durationMs = m_config.longRestDurationMs;
    else if (nextState == PieceState::short_rest)
        durationMs = m_config.shortRestDurationMs;

    if (durationMs > 0) {
        // מה ה-next_state של ה-rest state?
        const auto& restCfg = getConfig(code, nextStateName);
        PieceState afterRest = state_from_string(restCfg.next_state_when_finished);
        m_stateTimers[piece.get_pos()] = PieceStateTimer{afterRest, durationMs};
    }
}

// ─────────────────────────────────────────────
// tick — קידום timers, מחזיר עדכונים
// ─────────────────────────────────────────────
std::vector<StateTimerUpdate> PieceStateMachine::tick(int deltaMs) {
    std::vector<StateTimerUpdate> result;

    auto it = m_stateTimers.begin();
    while (it != m_stateTimers.end()) {
        it->second.remainingMs -= deltaMs;
        if (it->second.remainingMs <= 0) {
            Position pos = it->first;
            PieceState targetState = it->second.targetState;
            result.push_back({pos, targetState});
            it = m_stateTimers.erase(it);

            // Chain: if targetState has its own timer (e.g. short_rest → idle),
            // auto-create the next timer so the chain continues.
            if (targetState == PieceState::long_rest) {
                m_stateTimers[pos] = PieceStateTimer{
                    PieceState::idle, m_config.longRestDurationMs};
            } else if (targetState == PieceState::short_rest) {
                m_stateTimers[pos] = PieceStateTimer{
                    PieceState::idle, m_config.shortRestDurationMs};
            }
        } else {
            ++it;
        }
    }

    return result;
}

// ─────────────────────────────────────────────
// getConfig — lazy-load + cache
// ─────────────────────────────────────────────
const PieceStateConfig& PieceStateMachine::getConfig(
    const std::string& code, const std::string& stateName)
{
    std::string key = code + "/" + stateName;
    auto it = m_configCache.find(key);
    if (it != m_configCache.end())
        return it->second;

    PieceStateConfig cfg = load_piece_state_config(m_rootDir, code, stateName);
    auto result = m_configCache.emplace(key, std::move(cfg));
    return result.first->second;
}

// ─────────────────────────────────────────────
// ms_per_cell_from_speed
// ─────────────────────────────────────────────
int PieceStateMachine::ms_per_cell_from_speed(double speed_m_per_sec) const {
    if (speed_m_per_sec <= 0.0) return m_config.msPerCell;


    double cellSizeMeters = m_config.cellSizePixels / m_config.pixelsPerMeter;

    return static_cast<int>(
        (cellSizeMeters / speed_m_per_sec) * 1000.0
    );
}


