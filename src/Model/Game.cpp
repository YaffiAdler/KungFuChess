#include "Game.h"
#include "CommandInterpreter.h"
#include <sstream>
#include <iostream>

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
Game::Game(Board board, GameConfig config)
    : m_board(std::move(board)), m_config(config) {}

// ─────────────────────────────────────────────
//  pixel_to_cell — המרת פיקסלים לתא לוח
// ─────────────────────────────────────────────
std::optional<Position> Game::pixel_to_cell(int x, int y) const noexcept {
    if (x < 0 || y < 0) return std::nullopt;  // קואורדינטות שליליות — מחוץ ללוח
    int col = x / m_config.cellSizePixels;
    int row = y / m_config.cellSizePixels;
    Position p{row, col};
    if (m_board.is_valid_position(p)) {
        return p;
    }
    return std::nullopt;
}

// ─────────────────────────────────────────────
//  execute_command — parse via CommandInterpreter + dispatch
// ─────────────────────────────────────────────
CommandResult Game::execute_command(const std::string& cmdLine) {
    auto parsed = CommandInterpreter::parse(cmdLine);

    switch (parsed.type) {
    case CommandType::Click:
        return handle_click(parsed.arg1, parsed.arg2);

    case CommandType::Wait:
        return handle_wait(parsed.arg1);

    case CommandType::PrintBoard:
        return handle_print_board();

    case CommandType::Unknown: {
        // זיהוי: חוסר ארגומנטים או פקודה לא מוכרת
        std::string firstWord;
        {
            std::istringstream iss(parsed.raw);
            iss >> firstWord;
        }
        if (firstWord == "click" || firstWord == "wait" || firstWord == "print") {
            return {false, "Invalid arguments"};
        }
        return {false, "Unknown command"};
    }
    }

    return {false, "Unknown command"};
}

// ─────────────────────────────────────────────
//  handle_click — לוגיקת קליק
//
//  כללים (לפי סדר קדימות):
//  1. אין בחירה + קליק על כלי ← בחר
//  2. יש בחירה + קליק על כלי ידידותי ← החלף בחירה
//  3. יש בחירה + קליק על תא אחר ← בקש מהלך (הבחירה נשמרת!)
//  4. קליק מחוץ ללוח ← התעלם
//  5. קליק על תא ריק ללא בחירה ← התעלם
// ─────────────────────────────────────────────
CommandResult Game::handle_click(int x, int y) {
    auto cellOpt = pixel_to_cell(x, y);
    if (!cellOpt.has_value()) {
        // קליק מחוץ ללוח — התעלם
        return {true, ""};
    }

    Position clicked = *cellOpt;
    const auto& cell = m_board.at(clicked);

    // ── 1. אין כלי נבחר + קליק על כלי ← בחר ──
    if (!m_selectedPos.has_value()) {
        if (cell.has_value()) {
            m_selectedPos = clicked;
        }
        // קליק על תא ריק ללא בחירה ← התעלם
        return {true, ""};
    }

    // ── שמירה: בחירה מצביעה על תא ריק ← בטל ──
    Position from = *m_selectedPos;
    const auto& fromCell = m_board.at(from);
    if (!fromCell.has_value()) {
        m_selectedPos.reset();
        return {true, ""};
    }

    // ── 2. קליק על אותו תא ← התעלם (למנוע מהלך לעצמו) ──
    if (clicked == from) {
        return {true, ""};
    }

    PieceColor selectedColor = fromCell->get_color();

    // ── 3. קליק על כלי ידידותי ← החלף בחירה ──
    if (cell.has_value() && cell->get_color() == selectedColor) {
        m_selectedPos = clicked;
        return {true, ""};
    }

    // ── 4. קליק על תא אחר (ריק / אויב) ← בדוק חוקיות לפי MoveGenerator ──
    Piece movingPiece = *fromCell;

    // הפק את כל המהלכים הגולמיים לפי MovementRules
    auto rawMoves = movingPiece.get_raw_moves(m_board.rows(), m_board.cols());

    // בדוק אם היעד נמצא ברשימת המהלכים החוקיים
    bool isLegal = false;
    for (const auto& m : rawMoves) {
        if (m == clicked) {
            isLegal = true;
            break;
        }
    }

    if (!isLegal) {
        // מהלך לא חוקי — התעלם (הבחירה נשארת)
        return {true, ""};
    }

    m_board.remove(from);
    movingPiece.set_pos(clicked);
    movingPiece.mark_moved();
    m_board.place(std::move(movingPiece));

    // ✦ הבחירה נשארת על הכלי! (בהתאם לדרישה: "Do not deselect the piece after the move")
    m_selectedPos = clicked;

    return {true, ""};
}

// ─────────────────────────────────────────────
//  handle_wait — קידום שעון המשחק
// ─────────────────────────────────────────────
CommandResult Game::handle_wait(int ms) {
    m_gameClock += std::chrono::milliseconds(ms);
    return {true, ""};
}

// ─────────────────────────────────────────────
//  handle_print_board
// ─────────────────────────────────────────────
CommandResult Game::handle_print_board() {
    return {true, board_string()};
}

// ─────────────────────────────────────────────
//  board_string — גישה חיצונית
// ─────────────────────────────────────────────
std::string Game::board_string() const {
    std::ostringstream oss;
    for (int r = 0; r < m_board.rows(); ++r) {
        for (int c = 0; c < m_board.cols(); ++c) {
            if (c > 0) oss << ' ';

            const auto& cell = m_board.at(r, c);
            if (cell.has_value()) {
                std::string token = cell->to_token();
                // ✦ סמן כלי נבחר: [wK] במקום wK
                bool isSelected = m_selectedPos.has_value()
                    && m_selectedPos->row == r
                    && m_selectedPos->col == c;
                if (isSelected) {
                    oss << '[' << token << ']';
                } else {
                    oss << token;
                }
            } else {
                // תא ריק: "." או "[.]" (האחרון לא סביר)
                oss << ".";
            }
        }
        oss << '\n';
    }
    return oss.str();
}
