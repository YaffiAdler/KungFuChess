#pragma once
#include <string>

/// מודול פירוש פקודות טקסטואליות.
/// SRP: תפקיד יחיד — לתרגם טקסט → Command + ארגומנטים.
/// ללא תלות ב-Game, Board, או כל דבר אחר.
enum class CommandType {
    Unknown,
    Click,
    Wait,
    PrintBoard,
};

struct ParsedCommand final {
    CommandType type = CommandType::Unknown;
    int         arg1 = 0;   // x ל-click, ms ל-wait
    int         arg2 = 0;   // y ל-click
    std::string raw;        // הפקודה המקורית (לשגיאות)
};

class CommandInterpreter final {
public:
    /// מפרש שורת פקודה ומחזיר ParsedCommand.
    /// SRP: רק parsing, שום לוגיקת משחק.
    [[nodiscard]] static ParsedCommand parse(const std::string& line);
};
