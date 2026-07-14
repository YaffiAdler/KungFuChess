#include "Piece.h"
#include "PieceTypeRegistry.h"
#include "MoveGenerator.h"
#include <stdexcept>
#include <cctype>

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
Piece::Piece(PieceColor color, std::string typeId, Position pos)
    : m_color(color), m_typeId(std::move(typeId)), m_pos(pos) {}

// ─────────────────────────────────────────────
//  get_symbol — מה-Registry
// ─────────────────────────────────────────────
char Piece::get_symbol() const {
    const auto* def = PieceTypeRegistry::instance().find_by_id(m_typeId);
    return def ? def->symbol : '?';
}

// ─────────────────────────────────────────────
//  to_token
// ─────────────────────────────────────────────
std::string Piece::to_token() const {
    char c = (m_color == PieceColor::White) ? 'w' : 'b';
    return std::string{c, get_symbol()};
}

// ─────────────────────────────────────────────
//  get_raw_moves — גנרי לחלוטין.
//  מחפש את MovementRules מה-Registry ומפעיל את MoveGenerator.
// ─────────────────────────────────────────────
std::vector<Position> Piece::get_raw_moves(int numRows, int numCols) const {
    const auto* def = PieceTypeRegistry::instance().find_by_id(m_typeId);
    if (!def) return {};

    std::vector<Position> moves;
    for (const auto& rule : def->rules) {
        auto generated = MoveGenerator::generate(
            rule, m_pos, m_color, numRows, numCols, m_hasMoved);
        moves.insert(moves.end(), generated.begin(), generated.end());
    }
    return moves;
}
