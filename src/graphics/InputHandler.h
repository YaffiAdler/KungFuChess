#pragma once

class Controller;
class GameEngine;

/// טיפול באירועי קלט — מקלדת ועכבר.
///
/// SRP: תפקיד יחיד — קבלת אירועים גולמיים והעברתם ל-Controller.
/// חסר לוגיקת משחק, חסר PixelMapper, חסרה קבלת החלטות.
class InputHandler final {
public:
    explicit InputHandler(Controller& controller) noexcept;

    /// טיפול במקש מקלדת.
    /// @return true = המשך לולאה, false = צא
    [[nodiscard]] bool process_key(int key, GameEngine& engine);

    /// רישום קליק עכבר (נקרא מ-mouse callback)
    void register_click(int x, int y);

    /// טיפול בקליק ממתין, אם יש.
    /// @return true = היה קליק וטופל, false = לא היה קליק
    [[nodiscard]] bool process_click(GameEngine& engine);

    /// איפוס מצב קליקים
    void flush();

private:
    Controller& m_controller;

    // קליקים ממתינים (נכתבים מ-callback, נקראים מה-main loop)
    int  m_clickX   = -1;
    int  m_clickY   = -1;
    bool m_hasClick = false;
};
