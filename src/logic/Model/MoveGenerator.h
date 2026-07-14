#pragma once
#include "MovementRule.h"
#include "Position.h"
#include "PieceColor.h"
#include <vector>

/// SRP: תפקיד יחיד — לייצר מיקומים חוקיים לפי MovementRule.
/// לוגיקה גנרית לחלוטין, ללא שום ידע על סוגי כלים ספציפיים.
class MoveGenerator final {
public:
    /// מייצר את כל המיקומים האפשריים לפי כלל תנועה נתון.
    /// @param rule       כלל התנועה
    /// @param from       מיקום התחלתי
    /// @param color      צבע הכלי (נחוץ לכיוון קדימה — stepForward)
    /// @param numRows    גובה הלוח
    /// @param numCols    רוחב הלוח
    /// @param hasMoved   האם הכלי כבר זז (ל-2 צעדים של רגלי)
    [[nodiscard]] static std::vector<Position>
    generate(const MovementRule& rule,
             Position from, PieceColor color,
             int numRows, int numCols,
             bool hasMoved);
};
