#pragma once
#include "Piece.h"
#include <optional>
#include <string>

/// Factory Pattern — יוצר אובייקטי Piece מטוקנים.
/// SRP: תפקיד יחיד — לתרגם token → Piece.
/// לגמרי data-driven: משתמש ב-PieceTypeRegistry.
class PieceFactory final {
public:
    /// יוצר Piece מ-token (למשל "wK" → King לבן).
    /// מחזיר std::nullopt לתא ריק (".").
    /// זורק std::invalid_argument על token לא חוקי.
    [[nodiscard]] static std::optional<Piece>
    create(const std::string& token, int row, int col);

    /// בודק תקינות token מול ה-Registry
    [[nodiscard]] static bool is_valid_token(const std::string& token);
};
