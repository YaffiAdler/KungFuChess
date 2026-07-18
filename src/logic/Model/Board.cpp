#include "Board.h"
#include <stdexcept>
#include <sstream>

// ─────────────────────────────────────────────
// Constructor — לוח ריק
// ─────────────────────────────────────────────
Board::Board(int rows, int cols)
    : m_rows(rows), m_cols(cols), m_cells(static_cast<std::size_t>(rows * cols)) {}

// ─────────────────────────────────────────────
// גישה לפי אינדקסים
// ─────────────────────────────────────────────
const std::optional<Piece>& Board::at(int row, int col) const {
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
        throw std::out_of_range(
            "Board::at — position (" + std::to_string(row) + "," +
            std::to_string(col) + ") out of bounds [" +
            std::to_string(m_rows) + "x" + std::to_string(m_cols) + "]");
    }
    return m_cells[static_cast<std::size_t>(row * m_cols + col)];
}

std::optional<Piece>& Board::at(int row, int col) {
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
        throw std::out_of_range(
            "Board::at — position (" + std::to_string(row) + "," +
            std::to_string(col) + ") out of bounds [" +
            std::to_string(m_rows) + "x" + std::to_string(m_cols) + "]");
    }
    return m_cells[static_cast<std::size_t>(row * m_cols + col)];
}

// ─────────────────────────────────────────────
// גישה לפי Position
// ─────────────────────────────────────────────
const std::optional<Piece>& Board::at(Position p) const {
    return at(p.row, p.col);
}

std::optional<Piece>& Board::at(Position p) {
    return at(p.row, p.col);
}

// ─────────────────────────────────────────────
// place — הצבת כלי בלוח
// ─────────────────────────────────────────────
void Board::place(Piece piece) {
    Position pos = piece.get_pos();
    if (!is_valid_position(pos)) {
        throw std::out_of_range("Board::place — position out of range");
    }
    at(pos) = std::move(piece);
}

// ─────────────────────────────────────────────
// remove — הסרת כלי ממיקום
// ─────────────────────────────────────────────
void Board::remove(Position pos) {
    if (!is_valid_position(pos)) {
        throw std::out_of_range("Board::remove — position out of range");
    }
    at(pos) = std::nullopt;
}

// ─────────────────────────────────────────────
// is_valid_position — בדיקת גבולות
// ─────────────────────────────────────────────
bool Board::is_valid_position(Position p) const noexcept {
    return p.row >= 0 && p.row < m_rows &&
           p.col >= 0 && p.col < m_cols;
}

// ─────────────────────────────────────────────
// to_string — הדפסת לוח
// ─────────────────────────────────────────────
std::string Board::to_string(const Position* selected) const {
    std::ostringstream oss;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (c > 0) oss << ' ';

            bool isSelected = selected && *selected == Position{r, c};

            const auto& cell = at(r, c);
            if (cell.has_value()) {
                if (isSelected) oss << '[' << cell->to_token() << ']';
                else oss << cell->to_token();
            } else {
                if (isSelected) oss << "[.]";
                else oss << '.';
            }
        }
        oss << '\n';
    }
    return oss.str();
}
