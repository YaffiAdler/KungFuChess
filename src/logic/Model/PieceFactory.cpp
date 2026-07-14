#include "PieceFactory.h"
#include "PieceTypeRegistry.h"
#include <stdexcept>
#include <cctype>

std::optional<Piece> PieceFactory::create(const std::string& token, int row, int col) {
    if (token == ".") return std::nullopt;
    if (token.length() != 2) {
        throw std::invalid_argument("Invalid token: " + token);
    }

    char colorChar = static_cast<char>(std::tolower(static_cast<unsigned char>(token[0])));
    char pieceChar = static_cast<char>(std::toupper(static_cast<unsigned char>(token[1])));

    if (colorChar != 'w' && colorChar != 'b') {
        throw std::invalid_argument("Invalid color in token: " + token);
    }

    // ── Registry lookup — no switch! ──
    const auto* def = PieceTypeRegistry::instance().find_by_symbol(pieceChar);
    if (!def) {
        throw std::invalid_argument("Unknown piece type in token: " + token);
    }

    PieceColor color = (colorChar == 'w') ? PieceColor::White : PieceColor::Black;
    return Piece{color, def->id, Position{row, col}};
}

bool PieceFactory::is_valid_token(const std::string& token) {
    if (token == ".") return true;
    if (token.length() != 2) return false;

    char c = static_cast<char>(std::tolower(static_cast<unsigned char>(token[0])));
    if (c != 'w' && c != 'b') return false;

    char p = static_cast<char>(std::toupper(static_cast<unsigned char>(token[1])));
    return PieceTypeRegistry::instance().find_by_symbol(p) != nullptr;
}
