#include "SnapshotBuilder.h"
#include <algorithm>

// ─────────────────────────────────────────────
// extract_piece_info — Piece → PieceInfo
// ─────────────────────────────────────────────
PieceInfo SnapshotBuilder::extract_piece_info(const Piece& piece)
{
    PieceInfo info;
    info.typeId = piece.type_id();
    info.code   = piece.get_code();
    info.color  = piece.get_color();
    info.pos    = piece.get_pos();
    info.state  = piece.get_state();

    return info;
}

// ─────────────────────────────────────────────
// extract_motion_info — Motion → MotionInfo
// ─────────────────────────────────────────────
MotionInfo SnapshotBuilder::extract_motion_info(const Motion& motion) {
    double progress = (motion.totalMs > 0)
        ? static_cast<double>(motion.elapsedMs) / motion.totalMs
        : 1.0;
    if (progress > 1.0) progress = 1.0;

        MotionInfo info;
        info.piece    = extract_piece_info(motion.piece);
        info.from     = motion.from;
        info.to       = motion.to;
        info.progress = progress;

        return info;
}

// ─────────────────────────────────────────────
// build — הרכבת GameSnapshot מלא
// ─────────────────────────────────────────────
GameSnapshot SnapshotBuilder::build(
    const Board& board,
    const RealTimeArbiter& arbiter,
    const std::optional<Position>& selectedPos,
    GameState state,
    PieceColor currentTurn,
    const std::optional<PieceColor>& winner) const
{
    GameSnapshot snap;

    snap.boardRows = board.rows();
    snap.boardCols = board.cols();

    // ── כלים מהלוח ──
    for (int r = 0; r < board.rows(); ++r) {
        for (int c = 0; c < board.cols(); ++c) {
            const auto& cell = board.at(r, c);
            if (cell.has_value()) {
                snap.pieces.push_back(extract_piece_info(*cell));
            }
        }
    }

    // ── תנועות פעילות ──
    for (const auto& motion : arbiter.motions()) {
        snap.motions.push_back(extract_motion_info(motion));
    }

    // ── שאר השדות ──
    snap.selectedPos = selectedPos;
    snap.state       = state;
    snap.currentTurn = currentTurn;
    snap.winner      = winner;

    return snap;
}
