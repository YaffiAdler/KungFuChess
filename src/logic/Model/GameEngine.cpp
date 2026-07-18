#include "GameEngine.h"
#include "RuleEngine.h"

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
GameEngine::GameEngine(Board board, GameConfig config)
    : m_board(std::move(board)), m_config(config) {}

// ─────────────────────────────────────────────
// start_game — מעבר מ-Waiting ל-Playing
// ─────────────────────────────────────────────
void GameEngine::start_game() noexcept {
    if (m_state == GameState::Waiting) {
        m_state = GameState::Playing;
    }
}

// ─────────────────────────────────────────────
// select — בחירת כלי
// ─────────────────────────────────────────────
bool GameEngine::select(Position pos) {
    // במשחק Kung-Fu Chess אין תורות — כל שחקן יכול לבחור כלי בכל זמן
    if (m_state != GameState::Playing) return false;
    if (m_busy) return false;               // תנועה פעילה — אין בחירות חדשות
    if (!m_board.is_valid_position(pos)) return false;

    const auto& cell = m_board.at(pos);
    if (!cell.has_value()) return false;

    m_selectedPos = pos;
    return true;
}

// ─────────────────────────────────────────────
// validate_move — שלב 1: אימות חוקיות בלבד (const)
// ─────────────────────────────────────────────
MoveValidation GameEngine::validate_move(Position from, Position to) const {
    // מצב לא פעיל — דחה לפני RuleEngine
    if (m_state != GameState::Playing) {
        return {false, "game_not_active"};
    }
    if (m_state == GameState::GameOver) {
        return {false, "game_over"};
    }
    if (m_busy) {
        return {false, "motion_in_progress"};
    }

    // האצלה ל-RuleEngine
    RuleEngine ruleEngine(m_board);
    return ruleEngine.validate_move(from, to);
}

// ─────────────────────────────────────────────
// commit_move — שלב 2: ביצוע לוגי (נקרא בהגעה)
// ─────────────────────────────────────────────
MoveResult GameEngine::commit_move(Position from, Position to) {
    // ── 1. קריאת הכלי מהמקור ──
    auto& fromCell = m_board.at(from);
    if (!fromCell.has_value()) {
        m_busy = false;
        return {false, "source_empty"};
    }

    Piece movingPiece = std::move(*fromCell);
    m_board.remove(from);

    // ── 2. בדיקת הכאה ──
    auto& targetCell = m_board.at(to);
    bool wasCapture = targetCell.has_value();

    // ── 3. הצבת הכלי ביעד ──
    movingPiece.set_pos(to);
    movingPiece.mark_moved();

    // ── 4. קידום רגלי (promotion אוטומטי למלכה) ──
    if (movingPiece.type_id() == "pawn") {
        int promoRow = (movingPiece.get_color() == PieceColor::White) ? 0 : m_board.rows() - 1;
        if (to.row == promoRow) {
            movingPiece = Piece(movingPiece.get_color(), "queen", to);
        }
    }

    m_board.place(std::move(movingPiece));

    //לטיפול בהמשך לאן צריך להעביר
    // ── 5. בדיקת סיום (מלך הוכה) ──
    bool gameOver = false;
    if (wasCapture) {
        bool whiteKingFound = false;
        bool blackKingFound = false;
        for (int r = 0; r < m_board.rows(); ++r) {
            for (int c = 0; c < m_board.cols(); ++c) {
                const auto& cell = m_board.at(r, c);
                if (cell.has_value() && cell->type_id() == "king") {
                    if (cell->get_color() == PieceColor::White) whiteKingFound = true;
                    else blackKingFound = true;
                }
            }
        }
        if (!whiteKingFound || !blackKingFound) {
            gameOver = true;
            m_state   = GameState::GameOver;
            m_winner  = !whiteKingFound ? PieceColor::Black : PieceColor::White;
        }
    }

    // ── 6. סיום תנועה — busy נשאר true (מנוהל ע"י commit_move_with_state)
    // busy לא מתאפס כאן — הסמכות ב-commit_move_with_state

    // Kung-Fu Chess: אין תורות, לא מעבירים תור
    return {true, "ok", wasCapture, gameOver, false};
}

// ─────────────────────────────────────────────
// start_piece_motion — NEW
// ─────────────────────────────────────────────
int GameEngine::start_piece_motion(Position from, Position to) {
    if (m_busy) return 0;

    auto& fromCell = m_board.at(from);
    if (!fromCell.has_value()) return 0;

    Piece& piece = *fromCell;
    std::string code = piece.get_code();

    // טוען config של move
    const auto& cfg = get_piece_state_config(code, "move");

    // קובע state = move
    piece.set_state(PieceState::move);

    // מחשב msPerCell מ-speed_m_per_sec
    double speed = cfg.speed_m_per_sec;
    int msPerCell = (speed > 0.0)
        ? static_cast<int>(m_config.cellSizePixels / speed)
        : m_config.msPerCell;

    // busy = true (מונע מהלכים נוספים)
    m_busy = true;

    return msPerCell > 0 ? msPerCell : 1;  // לפחות 1ms
}

// ─────────────────────────────────────────────
// commit_move_with_state — NEW
// ─────────────────────────────────────────────
void GameEngine::commit_move_with_state(Position from, Position to) {
    // 1. ביצוע commit_move (מעדכן לוח, בודק game over)
    //    commit_move לא מבטל busy יותר
    MoveResult result = commit_move(from, to);
    if (!result.success) {
        m_busy = false;
        return;
    }

    // 2. PieceState → next_state מ-config
    auto& destCell = m_board.at(to);
    if (!destCell.has_value()) {
        m_busy = false;
        return;
    }

    Piece& piece = *destCell;
    std::string code = piece.get_code();

    // ה-state שעכשיו הסתיים (move / jump)
    // נניח שזה move כרגע
    const auto& moveCfg = get_piece_state_config(code, "move");
    std::string nextStateName = moveCfg.next_state_when_finished;
    PieceState nextState = state_from_string(nextStateName);

    piece.set_state(nextState);

    // 3. הגדרת timer ל-rest state → idle
    int durationMs = 0;
    if (nextState == PieceState::long_rest)
        durationMs = m_config.longRestDurationMs;
    else if (nextState == PieceState::short_rest)
        durationMs = m_config.shortRestDurationMs;

    if (durationMs > 0) {
        // מה ה-next_state של ה-rest state?
        const auto& restCfg = get_piece_state_config(code, nextStateName);
        PieceState afterRest = state_from_string(restCfg.next_state_when_finished);
        m_stateTimers[to] = PieceStateTimer{afterRest, durationMs};
    }

    // 4. סיום busy — מאפשר תנועה חדשה
    m_busy = false;
}

// ─────────────────────────────────────────────
// tick_state_machines — NEW
// ─────────────────────────────────────────────
void GameEngine::tick_state_machines(int deltaMs) {
    auto it = m_stateTimers.begin();
    while (it != m_stateTimers.end()) {
        it->second.remainingMs -= deltaMs;
        if (it->second.remainingMs <= 0) {
            auto& cellOpt = m_board.at(it->first);
            if (cellOpt.has_value()) {
                cellOpt->set_state(it->second.targetState);
            }
            it = m_stateTimers.erase(it);
        } else {
            ++it;
        }
    }
}

// ─────────────────────────────────────────────
// get_piece_state_config — NEW: load with cache
// ─────────────────────────────────────────────
const PieceStateConfig& GameEngine::get_piece_state_config(
    const std::string& code, const std::string& stateName)
{
    std::string key = code + "/" + stateName;
    auto it = m_pieceStateConfigCache.find(key);
    if (it != m_pieceStateConfigCache.end())
        return it->second;

    // טוען config.json ומכניס למטמון
    PieceStateConfig cfg = load_piece_state_config("src/graphics/pieces2", code, stateName);
    auto result = m_pieceStateConfigCache.emplace(key, std::move(cfg));
    return result.first->second;
}

// ─────────────────────────────────────────────
// ms_per_cell_from_speed — NEW helper
// ─────────────────────────────────────────────
int GameEngine::ms_per_cell_from_speed(double speed_m_per_sec) const {
    if (speed_m_per_sec <= 0.0) return m_config.msPerCell;
    return static_cast<int>(m_config.cellSizePixels / speed_m_per_sec);
}

// ─────────────────────────────────────────────
// move_selected_to — עוטף validate + commit עם לוגיקת בחירה
// ─────────────────────────────────────────────
MoveResult GameEngine::move_selected_to(Position target) {
    // ── 0. אין בחירה ──
    if (!m_selectedPos.has_value()) {
        return {false, "No piece selected"};
    }

    Position from = *m_selectedPos;
    const auto& fromCell = m_board.at(from);

    // שמירה: בחירה מצביעה על תא ריק ← בטל
    if (!fromCell.has_value()) {
        m_selectedPos.reset();
        return {false, ""};
    }

    // ── 1. קליק על אותו תא — התעלם ──
    if (target == from) {
        return {false, ""};
    }

    // ── 2. כלי ידידותי ביעד ← החלף בחירה ──
    PieceColor selectedColor = fromCell->get_color();
    const auto& targetCell = m_board.at(target);
    if (targetCell.has_value() && targetCell->get_color() == selectedColor) {
        m_selectedPos = target;
        return {true, "selected"};
    }

    // ── 3. אימות המהלך ──
    auto validation = validate_move(from, target);
    if (!validation.is_valid) {
        m_selectedPos.reset();
        return {false, validation.reason};
    }

    // ── 4. ביצוע מיידי (legacy mode — כשאין Arbiter) ──
    // כשמחובר ל-Arbiter, Controller יקרא ל-validate_move לחוד
    // וינהל את התנועה. move_selected_to בקונטקסט של Controller
    // לא יגיע לפה; הוא יטופל ב-handle_click.
    MoveResult result = commit_move(from, target);
    m_selectedPos.reset();
    return result;
}

// ─────────────────────────────────────────────
// snapshot
// ─────────────────────────────────────────────
GameSnapshot GameEngine::snapshot() const {
    return GameSnapshot{
        m_board,         // העתקה עמוקה
        m_selectedPos,
        std::nullopt,    // arbiterMotion — ימולא ע"י Controller
        m_gameClock,
        m_currentTurn
    };
}

// ─────────────────────────────────────────────
// restore
// ─────────────────────────────────────────────
void GameEngine::restore(const GameSnapshot& snap) {
    m_board        = snap.board;
    m_selectedPos  = snap.selectedPos;
    m_gameClock    = snap.gameClock;
    m_currentTurn  = snap.currentTurn;
    m_state        = GameState::Playing;
    m_winner.reset();
    m_busy         = false;
}
