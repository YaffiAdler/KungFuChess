#pragma once
#include <cstddef>
#include <cstddef>

struct Position final {
    int row;
    int col;

        [[nodiscard]] bool operator==(const Position& other) const noexcept {
        return row == other.row && col == other.col;
    }

    [[nodiscard]] bool is_valid(int num_rows, int num_cols) const noexcept {
        return row >= 0 && row < num_rows && col >= 0 && col < num_cols;
    }
    
};