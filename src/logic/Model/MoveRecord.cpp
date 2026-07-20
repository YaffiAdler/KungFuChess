#include "MoveRecord.h"

// ─────────────────────────────────────────────
// to_algebraic — ממיר MoveRecord לתווי שחמט אלגברי
// ─────────────────────────────────────────────
std::string to_algebraic(const MoveRecord& record, int numRows) {
    // שליפת סימבול הכלי: K=king, Q=queen, R=rook, B=bishop, N=knight, ""=pawn
    char symbol = record.piece.get_symbol(); // מחזיר 'K','Q','R','B','N','P'

    // המרת קואורדינטות: col→file (a-h), row→rank (8-1)
    char file = static_cast<char>('a' + record.to.col);
    char rank = static_cast<char>('1' + (numRows - 1 - record.to.row));

    std::string result;

    bool isPawn = (symbol == 'P');

    if (record.capture) {
        if (isPawn) {
            // הכאה של רגלי: קובץ המקור + 'x' + יעד
            result += static_cast<char>('a' + record.from.col);
        } else {
            // הכאה של כלי אחר: סימבול + 'x'
            result += symbol;
        }
        result += 'x';
    } else {
        // מהלך רגיל: סימבול (ללא רגלי) + יעד
        if (!isPawn) {
            result += symbol;
        }
    }

    result += file;
    result += rank;

    return result;
}
