#pragma once

/// קונפיגורציית משחק — כל הקבועים במקום אחד.
/// No hard-coded constants in business logic!
struct GameConfig final {
    int boardRows          = 8;     // גובה הלוח
    int boardCols          = 8;     // רוחב הלוח
    int cellSizePixels     = 100;   // גודל תא בפיקסלים (לפקודת click)
    int msPerCell          = 1000;  // אלפיות שנייה לצעד-תא (fallback)
    double pixelsPerMeter = 100.0;
    // ── State durations ──
    int jumpDurationMs      = 800;    // זמן קפיצה במקום (jump → short_rest)
    int longRestDurationMs  = 2000;   // זמן מנוחה ארוכה (long_rest → idle)
    int shortRestDurationMs = 1500;   // זמן מנוחה קצרה (short_rest → idle)
    int sidePanelWidth      = 200;   // רוחב פאנל צדדי (להצגת היסטוריית מהלכים)
};
