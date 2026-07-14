#pragma once

#include "../logic/Model/PixelMapper.h"
#include "../logic/Model/GameEngine.h"
#include "../logic/Model/Position.h"
#include <atomic>
#include <optional>

/// טיפול בקלט משתמש: עכבר ומקלדת.
///
/// SRP: תפקיד יחיד — ללכוד אירועי קלט ולתרגם אותם לפעולות GameEngine.
///     - עכבר: בחירה / הזזה (רק במצב Playing)
///     - מקלדת: Enter = התחלה, q/ESC = יציאה
class InputHandler final {
public:
    InputHandler(const PixelMapper& mapper);

    /// רישום קליק עכבר (נקרא מ-mouse callback)
    void register_click(int x, int y);

    /// עיבוד קליק ממתין, אם יש. מחזיר true אם היה קליק וטופל.
    /// @return true = קליק טופל, false = לא היה קליק
    bool process_click(GameEngine& engine);

    /// עיבוד מקש מקלדת. מחזיר true אם יש להמשיך את הלולאה.
    /// @return true = המשך, false = צא מהלולאה
    bool process_key(int key, GameEngine& engine);

    /// איפוס מצב קליקים (ללא עיבוד)
    void flush_click();

private:
    const PixelMapper& m_mapper;

    // קליקים ממתינים (נכתבים מ-callback, נקראים מה-main loop)
    std::atomic<int>  m_clickX{-1};
    std::atomic<int>  m_clickY{-1};
    std::atomic<bool> m_hasClick{false};
};
