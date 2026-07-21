#pragma once
#include "GameSnapshot.h"
#include "../Model/Board.h"
#include "../Model/Piece.h"
#include "../Realtime/RealTimeArbiter.h"
#include "../Model/GameState.h"
#include <optional>

/// בנאי GameSnapshot — ה-class היחיד שמתרגם אובייקטי Domain ל-DTO.
///
/// SRP: SnapshotBuilder קורא את מצב המשחק (Board, Arbiter, GameState…)
/// וממיר אותו ל-GameSnapshot — DTO שטוח ללא מצביעים לאובייקטי Domain.
///
/// שכבת הגרפיקה מקבלת רק GameSnapshot, ולא נוגעת ב-Board/Piece/Motion.
class SnapshotBuilder final {
public:
    SnapshotBuilder() = default;

    /// בניית GameSnapshot מלא מתוך מצב המערכת.
    /// @param board         הלוח הנוכחי
    /// @param arbiter       ארביטר התנועות (לחילוץ MotionInfo)
    /// @param selectedPos   תא נבחר (מה-Engine)
    /// @param state         מצב המשחק
    /// @param currentTurn   תור נוכחי
    /// @param winner        מנצח (אם יש)
    [[nodiscard]] GameSnapshot build(
        const Board& board,
        const RealTimeArbiter& arbiter,
        const std::optional<Position>& selectedPos,
        GameState state,
        PieceColor currentTurn,
        const std::optional<PieceColor>& winner) const;

private:
    /// המרת Piece בודד ל-PieceInfo
    [[nodiscard]] static PieceInfo extract_piece_info(const Piece& piece);

    /// המרת Motion בודד ל-MotionInfo (כולל חישוב progress)
    [[nodiscard]] static MotionInfo extract_motion_info(const Motion& motion);
};
