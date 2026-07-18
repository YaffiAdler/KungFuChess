#pragma once
#include "Board.h"
#include "Position.h"

struct MovementRule;

/// PieceRules: only checks blocking (path clearing).
/// Capture logic belongs in RuleEngine.
class PieceRules {
public:
    [[nodiscard]] static bool is_path_clear(
        const Board& board, const Piece& piece,
        Position from, Position to) noexcept;

    [[nodiscard]] static const char* last_block_reason() noexcept { return m_lastReason; }

private:
    static const char* m_lastReason;
    static bool slide_path_clear(const Board& board, Position from, Position to);
    static bool step_path_clear(const Board& board, Position from, Position to, int maxSteps);
};
