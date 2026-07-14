#include "RuleEngine.h"
#include <algorithm>
#include <iostream>

MoveValidation RuleEngine::validate_move(Position from, Position to) const noexcept {
    // ── 1. מקור מחוץ ללוח ──
    if (!m_board->is_valid_position(from)) {
        return {false, "outside_board"};
    }

    // ── 2. יעד מחוץ ללוח ──
    if (!m_board->is_valid_position(to)) {
        return {false, "outside_board"};
    }

    // ── 3. תא מקור ריק ──
    const auto& srcCell = m_board->at(from);
    if (!srcCell.has_value()) {
        return {false, "empty_source"};
    }

    const Piece& piece = *srcCell;

    // ── 4. מהלך לאותו תא ──
    if (from == to) {
        return {false, "illegal_piece_move"};
    }

    // ── 5. יעד תפוס על ידי כלי ידידותי ──
    const auto& dstCell = m_board->at(to);
    if (dstCell.has_value() && dstCell->get_color() == piece.get_color()) {
        return {false, "friendly_destination"};
    }

    // ── 6. שאילת כלל תנועה — האצלה ל-MoveGenerator ──
    auto rawMoves = piece.get_raw_moves(m_board->rows(), m_board->cols());
    std::cerr << "DEBUG RuleEngine: from=" << from.row << "," << from.col
              << " to=" << to.row << "," << to.col
              << " rawMoves.size=" << rawMoves.size();
    for (const auto& m : rawMoves) {
        std::cerr << " [" << m.row << "," << m.col << "]";
    }
    std::cerr << std::endl;
    bool found = std::find(rawMoves.begin(), rawMoves.end(), to) != rawMoves.end();

    if (!found) {
        std::cerr << "DEBUG RuleEngine: to NOT in rawMoves — illegal_piece_move" << std::endl;
        return {false, "illegal_piece_move"};
    }

    return {true, "ok"};
}
