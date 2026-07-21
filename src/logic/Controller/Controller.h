#pragma once
#include "PixelMapper.h"
#include "../Model/Position.h"
#include "../Engine/GameEngine.h"
#include "../Realtime/RealTimeArbiter.h"
#include <optional>
#include <string>

class GameEngine;

/// בקר — מתרגם פעולות משתמש (קליקים) לפקודות משחק + ניהול זמן.
///
/// SRP: תפקיד יחיד — תרגום קלט גולמי לפעולות GameEngine, בתיווך RealTimeArbiter.
/// הבקר אינו מחליט על חוקיות שחמט; הוא שואל את GameEngine.
///
/// אחריות:
///   • קבלת קואורדינטות קליק
///   • המרת פיקסלים לתאי לוח באמצעות PixelMapper
///   • שמירת כלי-נבחר
///   • בקליק ראשון — בחירת כלי
///   • בקליק שני — אימות דרך GameEngine, התחלת תנועה ב-Arbiter
///   • tick — קידום זמן Arbiter, commit_move בהגעה
///   • ניקוי הבחירה
class Controller final {
public:
    explicit Controller(const GameConfig& config) noexcept;

    /// חיבור ל-GameEngine (נעשה אחרי הבנייה)
    void set_engine(GameEngine* engine) noexcept { m_engine = engine; }

    /// טיפול בקליק עכבר. מחזיר תיאור טקסטואלי של הפעולה (ללוג).
    /// @param x קואורדינטת X של הקליק (פיקסלים)
    /// @param y קואורדינטת Y של הקליק (פיקסלים)
    /// @return תיאור הפעולה ("ok", "selected", "deselected", "jump", וכו')
    [[nodiscard]] std::string handle_click(int x, int y);

    /// טיפול במקש — ESC/q = צא, ENTER = התחלה/הפעלה מחדש.
    /// @return false = יציאה מהמשחק
    [[nodiscard]] bool handle_key(int key);

    /// קידום זמן: Arbiter.tick(), GameEngine.tick(), commit_move.
    void tick(int deltaMs);

    /// ביטול בחירה נוכחית
    void cancel_selection() noexcept;

    /// גישה ל-PixelMapper (עבור InputHandler)
    [[nodiscard]] PixelMapper& mapper() noexcept { return m_mapper; }

    /// גישה ל-Arbiter (עבור Renderer / Window)
    [[nodiscard]] RealTimeArbiter& arbiter() noexcept { return m_arbiter; }
    [[nodiscard]] const RealTimeArbiter& arbiter() const noexcept { return m_arbiter; }

private:
    PixelMapper            m_mapper;
    RealTimeArbiter        m_arbiter;
    std::optional<Position> m_selected;
    GameEngine*            m_engine = nullptr;
};
