#pragma once
#include <vector>

/// סוג תנועה גנרי — Step (צעד), Slide (החלקה), Jump (קפיצה)
enum class MovePattern {
    Step,   // צעד בודד בכיוונים נתונים
    Slide,  // החלקה עד חסימה
    Jump    // קפיצה קבועה (L של פרש)
};

/// וקטור כיוון
struct Direction {
    int dr;  // שינוי בשורה (שלילי = למעלה)
    int dc;  // שינוי בעמודה (שלילי = שמאלה)
};

/// כלל תנועה — DATA, לא CODE.
/// מגדיר איך כלי יכול לנוע: סוג התנועה, כיוונים, האם מותר להכות.
struct MovementRule final {
    MovePattern pattern;
    std::vector<Direction> directions;
    int  maxSteps   = 1;     // מספר צעדים מקסימלי; -1 = ללא הגבלה (Slide)
    bool canCapture = true;  // האם מותר להכות בתנועה זו

    // ─── Factory methods ───
    static MovementRule step(std::vector<Direction> dirs);
    static MovementRule slide(std::vector<Direction> dirs);
    static MovementRule jump(std::vector<Direction> offsets);
};
