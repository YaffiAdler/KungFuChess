#pragma once

#include "Piece.h"
#include "Position.h"
#include <vector>
#include <optional>
#include <string>
#include <memory>

/// לוח השחמט — מכיל תאים (std::optional<Piece>).
/// משתמש ב-optional במקום unique_ptr — פשוט יותר, עותק אפשרי.
class Board final {
public:
    Board(int rows, int cols);

    // ─── ממדים ───
    [[nodiscard]] int rows() const noexcept { return m_rows; }
    [[nodiscard]] int cols() const noexcept { return m_cols; }

    // ─── גישה ישירה ───
    [[nodiscard]] const std::optional<Piece>& at(int row, int col) const;
    [[nodiscard]] std::optional<Piece>&       at(int row, int col);
    [[nodiscard]] const std::optional<Piece>& at(Position p) const;
    [[nodiscard]] std::optional<Piece>&       at(Position p);

    /// הצבת כלי בלוח
    void place(Piece piece);

    /// הסרת כלי ממיקום
    void remove(Position pos);

    /// בדיקת גבולות
    [[nodiscard]] bool is_valid_position(Position p) const noexcept;

    /// מעבר על כל התאים (עבור UI / Parser)
    [[nodiscard]] const auto& cells() const noexcept { return m_cells; }

    /// הדפסת לוח
    [[nodiscard]] std::string to_string() const;

private:
    int m_rows;
    int m_cols;
    std::vector<std::optional<Piece>> m_cells;
};
