#pragma once

/// קונפיגורציית משחק — כל הקבועים במקום אחד.
/// No hard-coded constants in business logic!
struct GameConfig final {
    int boardRows      = 8;    // גובה הלוח
    int boardCols      = 8;    // רוחב הלוח
    int cellSizePixels = 100;  // גודל תא בפיקסלים (לפקודת click)
};
