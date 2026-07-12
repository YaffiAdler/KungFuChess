#include "Board.h"
#include <stdexcept>
#include <sstream>

// ─────────────────────────────────────────────
//  Constructor — לוח ריק
// ─────────────────────────────────────────────
Board::Board(int rows, int cols)
    : m_rows(rows), m_cols(cols), m_cells(static_cast<std::size_t>(rows * cols)) {}

// ─────────────────────────────────────────────
//  גישה לפי אינדקסים
// ─────────────────────────────────────────────
const std::optional<Piece>& Board::at(int row, int col) const {
    return m_cells[static_cast<std::size_t>(row * m_cols + col)];
}

std::optional<Piece>& Board::at(int row, int col) {
    return m_cells[static_cast<std::size_t>(row * m_cols + col)];
}

const std::optional<Piece>& Board::at(Position p) const {
    return at(p.row, p.col);
}

std::optional<Piece>& Board::at(Position p) {
    return at(p.row, p.col);
}

// ─────────────────────────────────────────────
//  place — מציב כלי בלוח
// ─────────────────────────────────────────────
void Board::place(Piece piece) {
    Position p = piece.get_pos();
    if (!is_valid_position(p)) {
        throw std::out_of_range("Board::place — position out of range");
    }
    at(p) = std::move(piece);
}

// ─────────────────────────────────────────────
//  remove — מסיר כלי ממיקום
// ─────────────────────────────────────────────
void Board::remove(Position pos) {
    if (!is_valid_position(pos)) {
        throw std::out_of_range("Board::remove — position out of range");
    }
    at(pos) = std::nullopt;
}

// ─────────────────────────────────────────────
//  עזר: בדיקת גבולות
// ─────────────────────────────────────────────
bool Board::is_valid_position(Position p) const noexcept {
    return p.row >= 0 && p.row < m_rows && p.col >= 0 && p.col < m_cols;
}

// ─────────────────────────────────────────────
//  to_string — ייצוג טקסטואלי
// ─────────────────────────────────────────────
std::string Board::to_string(const Position* selected) const {
    std::ostringstream oss;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (c > 0) oss << ' ';
            const auto& cell = at(r, c);
            if (cell.has_value()) {
                std::string token = cell->to_token();
                bool isSelected = selected
                    && selected->row == r
                    && selected->col == c;
                if (isSelected) {
                    oss << '[' << token << ']';
                } else {
                    oss << token;
                }
            } else {
                oss << ".";
            }
        }
        oss << '\n';
    }
    return oss.str();
}
