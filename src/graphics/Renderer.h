#pragma once

#include "../logic/Model/GameConfig.h"
#include "../logic/Model/GameSnapshot.h"
#include "../logic/Model/Position.h"
#include "../logic/Model/RealTimeArbiter.h"
#include "../logic/Model/MoveRecord.h"
#include "PieceRenderer.h"
#include "img.hpp"
#include <string>
#include <vector>
#include <chrono>

/// ציור מלא של המסך: לוח + כלים + overlay מצב + סימון בחירה + היסטוריית מהלכים.
///
/// SRP: תפקיד יחיד — לתרגם את מצב ה-GameSnapshot + Arbiter לתמונה על המסך.
/// אינו מכיר קלט, לולאת משחק, או OpenCV windows.
class Renderer final {
public:
    /// @param boardImagePath נתיב לתמונת הלוח (למשל "src/graphics/board.png")
    /// @param piecesRootDir תיקיית sprites של הכלים
    /// @param cellWidth רוחב תא ראשוני (יעודכן אוטומטית כשהלוח נטען)
    /// @param cellHeight גובה תא ראשוני
    Renderer(const std::string& boardImagePath,
             const std::string& piecesRootDir,
             int cellWidth, int cellHeight);

    /// טוען את תמונת הלוח ומתאים את גדלי התאים
    /// @param snapshot תמונת מצב התחלתית
    /// @param panelWidth רוחב הפאנלים הצדדיים (0 = אין פאנלים)
    void init(const GameSnapshot& snapshot, int panelWidth);

    /// ציור מלא של פריים אחד על המסך
    /// @param screen buffer המסך
    /// @param snapshot תמונת מצב המשחק
    /// @param arbiter מנהל התנועות (לאינטרפולציה)
    void render_frame(Img& screen, const GameSnapshot& snapshot,
                      const RealTimeArbiter& arbiter);

    /// גישה ל-PieceRenderer
    [[nodiscard]] PieceRenderer& piece_renderer() noexcept { return m_pieceRenderer; }

    /// קידום אנימציות ספרייט
    void tick_animations(int deltaMs) {
        m_pieceRenderer.advance_animations(deltaMs);
    }

    /// רוחב מסך כולל (לוח + פאנלים)
    [[nodiscard]] int total_width() const noexcept {
        return m_boardImage.get_mat().cols + 2 * m_panelWidth;
    }

    /// גובה המסך
    [[nodiscard]] int total_height() const noexcept {
        return m_boardImage.get_mat().rows;
    }

private:
    void draw_motion_piece(Img& screen, const Motion& motion);
    void draw_waiting_overlay(Img& screen);
    void draw_gameover_overlay(Img& screen, const GameSnapshot& snapshot);
    void draw_selection_marker(Img& screen, Position pos, int cellW, int cellH);
    void draw_move_history(Img& screen, const std::vector<MoveRecord>& history);

    /// כתיבת פאנל צדדי בודד (שחור או לבן)
    void draw_single_panel(Img& screen,
                           const std::vector<MoveRecord>& allMoves,
                           PieceColor panelColor,
                           int panelX, int panelWidth);

    /// תיעוד זמני סיום למהלכים חדשים (שכבת גרפיקה)
    void capture_timestamps(const std::vector<MoveRecord>& history);

    Img            m_boardImage;
    PieceRenderer  m_pieceRenderer;
    int m_cellWidth;
    int m_cellHeight;
    int m_panelWidth = 0;

    /// חותמות זמן למהלכים (מקביל ל-moveHistory — כל ערך מתאים למהלך באותו אינדקס)
    std::vector<std::string> m_moveTimestamps;
};
