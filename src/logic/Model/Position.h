#pragma once

#include <string>

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

