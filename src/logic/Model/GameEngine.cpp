#include "GameEngine.h"
#include "RuleEngine.h"

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
GameEngine::GameEngine(Board board, GameConfig config,
                        std::string piecesRootDir)
    : m_board(std::move(board))
    , m_config(config)
    , m_stateMachine(config, std::move(piecesRootDir))
    , m_moveExecutor(m_board)
{}

// ─────────────────────────────────────────────
// start_game — מעבר מ-Waiting ל-Playing
// ─────────────────────────────────────────────
void GameEngine::start_game() noexcept {
    if (m_state == GameState::Waiting) {
        m_moveHistory.clear();
        m_state = GameState::Playing;
    }
}

// ─────────────────────────────────────────────
// select — בחירת כלי
// ─────────────────────────────────────────────
bool GameEngine::select(Position pos) {
    if (m_state != GameState::Playing) return false;

    if (!m_board.is_valid_position(pos)) return false;

    const auto& cell = m_board.at(pos);
    if (!cell.has_value()) return false;

    m_selectedPos = pos;
    return true;
}

// ─────────────────────────────────────────────
// validate_move — האצלה ל-RuleEngine (const)
// ─────────────────────────────────────────────
MoveValidation GameEngine::validate_move(Position from, Position to) const {
    if (m_state != GameState::Playing) {
        return {false, "game_not_active"};
    }
    if (m_state == GameState::GameOver) {
        return {false, "game_over"};
    }

    RuleEngine ruleEngine(m_board);
    return ruleEngine.validate_move(from, to);
}

// ─────────────────────────────────────────────
// commit_move — האצלה ל-MoveExecutor
// ─────────────────────────────────────────────
MoveResult GameEngine::commit_move(Position from, Position to) {
    if (m_state != GameState::Playing) {
        return {false, "game_not_active"};
    }

    // תיעוד הכלי שהוכה (אם יש) לפני הביצוע
    std::optional<Piece> capturedPiece;
    const auto& targetCell = m_board.at(to);
    if (targetCell.has_value()) {
        capturedPiece = *targetCell;
    }

    // תיעוד הכלי המזיז לפני ההסרה מהלוח
    const auto& fromCell = m_board.at(from);
    if (!fromCell.has_value()) {
        return {false, "source_empty"};
    }
    Piece movingPiece = *fromCell;          // העתק
    PieceColor player = movingPiece.get_color();

    auto result = m_moveExecutor.execute(from, to);
    if (!result.success) {
        return {false, "execute_failed"};
    }

    // תיעוד המהלך בהיסטוריה
    m_moveHistory.push_back(MoveRecord{
        from,
        to,
        std::move(movingPiece),
        result.capture,
        capturedPiece,
        player
    });

    if (result.gameOver) {
        m_state = GameState::GameOver;
        m_winner = *result.winner;
    }

    return {true, "ok", result.capture, result.gameOver, false};
}

// ─────────────────────────────────────────────
// startJump — idle → jump + timer
// ─────────────────────────────────────────────
bool GameEngine::startJump(Position pos) {
    if (m_state != GameState::Playing) return false;

    auto& cell = m_board.at(pos);
    if (!cell.has_value()) return false;

    bool started = m_stateMachine.startJump(*cell);
    if (started) {
        m_jumpingPos = pos;
    }
    return started;
}

// ─────────────────────────────────────────────
// startMotion — האצלה ל-PieceStateMachine
// ─────────────────────────────────────────────
int GameEngine::startMotion(Position from, Position to) {
    auto& fromCell = m_board.at(from);
    if (!fromCell.has_value()) return 0;

    Piece& piece = *fromCell;
    int msPerCell = m_stateMachine.startMotion(piece, from, to);
    return msPerCell;
}

// ─────────────────────────────────────────────
// tick — אורקסטרציה מלאה (רב-תנועתי)
// ─────────────────────────────────────────────
bool GameEngine::tick(int deltaMs, RealTimeArbiter& arbiter) {
    bool anyCompleted = false;

    // 1. קידום state timers (rest → idle, jump → short_rest → idle)
    auto updates = m_stateMachine.tick(deltaMs);
    for (auto& upd : updates) {
        auto& cell = m_board.at(upd.pos);
        if (cell.has_value()) {
            cell->set_state(upd.newState);
        }

        // קפיצה הסתיימה סופית (jump → short_rest → idle)
        if (m_jumpingPos.has_value() && upd.pos == *m_jumpingPos &&
            upd.newState == PieceState::idle) {
            m_jumpingPos.reset();
        }
    }

    // 2. Arbiter — קידום כל התנועות
    if (!arbiter.hasActiveMotion()) return false;

    auto completedMotions = arbiter.tick(deltaMs);

    // 3. ביצוע כל תנועה שהסתיימה
    for (auto& motion : completedMotions) {
        // תיעוד הכלי שהוכה (אם יש) לפני הביצוע
        std::optional<Piece> capturedPiece;
        const auto& targetCell = m_board.at(motion.to);
        if (targetCell.has_value()) {
            capturedPiece = *targetCell;
        }

        PieceColor player = motion.piece.get_color();
        auto execResult = m_moveExecutor.execute(motion.from, motion.to);

        // 4. State transition + timer + תיעוד מהלך
        if (execResult.success) {
            // תיעוד המהלך בהיסטוריה
            m_moveHistory.push_back(MoveRecord{
                motion.from,
                motion.to,
                std::move(motion.piece),
                execResult.capture,
                capturedPiece,
                player
            });

            auto& destCell = m_board.at(motion.to);
            if (destCell.has_value()) {
                m_stateMachine.completeMotion(
                    *destCell, destCell->get_code(), "move");
            }
            if (execResult.gameOver) {
                m_state = GameState::GameOver;
                m_winner = *execResult.winner;
            }
        }

        anyCompleted = true;
    }

    return anyCompleted;
}

// ─────────────────────────────────────────────
// move_selected_to — legacy / tests (ללא Arbiter)
// ─────────────────────────────────────────────
MoveResult GameEngine::move_selected_to(Position target) {
    if (!m_selectedPos.has_value()) {
        return {false, "nothing_selected"};
    }

    Position from = *m_selectedPos;
    const auto& fromCell = m_board.at(from);

    if (!fromCell.has_value()) {
        m_selectedPos.reset();
        return {false, "source_disappeared"};
    }

    PieceColor movingColor = fromCell->get_color();
    const auto& targetCell = m_board.at(target);
    if (targetCell.has_value() &&
        targetCell->get_color() == movingColor) {
        // בחר כלי אחר
        m_selectedPos = target;
        return {true, "selected"};
    }

    // אימות המהלך
    auto validation = validate_move(from, target);
    if (!validation.is_valid) {
        m_selectedPos.reset();
        return {false, validation.reason};
    }

    // ביצוע (ללא Arbiter)
    auto result = m_moveExecutor.execute(from, target);
    m_selectedPos.reset();

    if (result.gameOver) {
        m_state = GameState::GameOver;
        m_winner = *result.winner;
    }

    return {result.success, result.success ? "ok" : "move_failed",
            result.capture, result.gameOver, false};
}

// ─────────────────────────────────────────────
// snapshot
// ─────────────────────────────────────────────
GameSnapshot GameEngine::snapshot() const {
    return GameSnapshot{
        m_board,
        m_selectedPos,
        {},  // arbiterMotions — empty; Controller/Window fills from arbiter
        m_gameClock,
        m_currentTurn,
        m_state,
        m_winner,
        m_moveHistory
    };
}

// ─────────────────────────────────────────────
// restore
// ─────────────────────────────────────────────
void GameEngine::restore(const GameSnapshot& snap) {
    m_board = snap.board;
    m_selectedPos = snap.selectedPos;
    m_gameClock = snap.gameClock;
    m_currentTurn = snap.currentTurn;
    m_state = snap.state;
    m_winner = snap.winner;
    m_moveHistory = snap.moveHistory;
    m_jumpingPos.reset();
}
