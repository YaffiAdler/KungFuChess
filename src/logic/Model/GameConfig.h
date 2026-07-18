#pragma once

/// קונפיגורציית משחק — כל הקבועים במקום אחד.
/// No hard-coded constants in business logic!
struct GameConfig final {
    int boardRows          = 8;     // גובה הלוח
    int boardCols          = 8;     // רוחב הלוח
    int cellSizePixels     = 100;   // גודל תא בפיקסלים (לפקודת click)
    int msPerCell          = 1000;  // אלפיות שנייה לצעד-תא (fallback)

    // ── State durations ──
    int longRestDurationMs  = 3000;   // זמן מנוחה ארוכה (long_rest → idle)
    int shortRestDurationMs = 1500;   // זמן מנוחה קצרה (short_rest → idle)
};
