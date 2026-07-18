#pragma once
#include "PixelMapper.h"
#include "../Model/Position.h"
#include "../Model/GameConfig.h"
#include "../Model/RealTimeArbiter.h"
#include <optional>
#include <string>

class GameEngine;

/// בקר — מתרגם פעולות משתמש (קליקים) לפקודות משחק + ניהול זמן.
///
/// SRP: תפקיד יחיד — תרגום קלט גולמי לפעולות GameEngine, בתיווך RealTimeArbiter.
/// הבקר אינו מחליט על חוקיות שחמט; הוא שואל את GameEngine.
///
/// אחריות:
/// • קבלת קואורדינטות קליק
/// • המרת פיקסלים לתאי לוח באמצעות PixelMapper
/// • שמירת כלי-נבחר
/// • בקליק ראשון — בחירת כלי
/// • בקליק שני — אימות דרך GameEngine, התחלת תנועה ב-Arbiter
/// • tick — קידום זמן Arbiter, commit_move בהגעה
/// • ניקוי הבחירה
class Controller final {
public:
    explicit Controller(const GameConfig& config) noexcept;

    /// טיפול בקליק עכבר. מחזיר תיאור טקסטואלי של הפעולה (ללוג).
    /// @param x קואורדינטת X של הקליק (פיקסלים)
    /// @param y קואורדינטת Y של הקליק (פיקסלים)
    /// @param engine מנוע המשחק
    /// @return מחרוזת תיאור
    std::string handle_click(int x, int y, GameEngine& engine);

    /// טיפול במקש מקלדת.
    /// @return true = המשך לולאה, false = צא
    bool handle_key(int key, GameEngine& engine);

    /// קידום זמן — state machines + Arbiter.tick(), ואם הסתיים → commit_move_with_state.
    /// @param deltaMs אלפיות שנייה שחלפו מאז הפריים הקודם
    /// @param engine מנוע המשחק
    void tick(int deltaMs, GameEngine& engine);

    /// גישה ל-Arbiter (עבור Renderer — אינטרפולציה, ועבור Snapshot)
    [[nodiscard]] const RealTimeArbiter& arbiter() const noexcept { return m_arbiter; }
    [[nodiscard]] RealTimeArbiter&       arbiter()       noexcept { return m_arbiter; }

    /// ביטול בחירה
    void cancel_selection() noexcept;

    /// גישה ל-PixelMapper (עבור InputHandler)
    [[nodiscard]] PixelMapper& mapper() noexcept { return m_mapper; }

private:
    PixelMapper       m_mapper;
    RealTimeArbiter   m_arbiter;
    std::optional<Position> m_selected;
};
