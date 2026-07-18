#pragma once

#include <string>
#include <functional>

// מייצג מיקום לוגי על הלוח (שורה ועמודה).
// Position הוא Value Object ואינו מכיר את גודל הלוח,
// כללי התנועה או קואורדינטות פיקסלים.
struct Position final {
    int row; // אינדקס השורה
    int col; // אינדקס העמודה

    // השוואת שני מיקומים.
    // מחזיר true אם גם השורה וגם העמודה זהות.
    [[nodiscard]]
    bool operator==(const Position& other) const noexcept {
        return row == other.row && col == other.col;
    }

    // מחזיר ייצוג קריא של המיקום.
    // דוגמה: "(2, 5)"
    [[nodiscard]]
    std::string to_string() const {
        return "(" + std::to_string(row) + ", " + std::to_string(col) + ")";
    }
};

// hash support for std::unordered_map<Position, ...>
namespace std {
template<>
struct hash<Position> {
    size_t operator()(const Position& p) const noexcept {
        return (static_cast<size_t>(p.row) << 16) ^ static_cast<size_t>(p.col);
    }
};
}

