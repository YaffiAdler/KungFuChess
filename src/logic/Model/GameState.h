#pragma once

/// מצבי המשחק הראשיים
enum class GameState {
    Waiting,   // טרם התחיל — כלים מוצגים, ממתינים לפקודת התחלה
    Playing,   // משחק פעיל
    GameOver   // הסתיים
};
