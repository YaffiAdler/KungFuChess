#pragma once

#include "../Model/Board.h"
#include <istream>
#include <string>
#include <optional>

/// מפענח קלט טקסטואלי של לוח שחמט.
///
/// פורמט:
///   Board:
///   wR wN wB wQ wK wB wN wR
///   wP wP wP wP wP wP wP wP
///   .  .  .  .  .  .  .  .
///   ...
///   Commands:
///   ...
class BoardParser final {
public:
    /// קורא לוח מ-istream. מחזיר std::nullopt במקרה של שגיאה.
    /// שגיאות נכתבות ל-errStream.
    [[nodiscard]] static std::optional<Board>
    parse(std::istream& in, std::ostream& errStream);

    /// קורא לוח מ-istream. זורק std::runtime_error בשגיאה.
    [[nodiscard]] static Board
    parse_or_throw(std::istream& in);

    /// מחרוזת שגיאה אחרונה
    [[nodiscard]] static const std::string& last_error() noexcept { return m_lastError; }

private:
    static std::string m_lastError;
};
