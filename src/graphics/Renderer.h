#pragma once

#include "../logic/Model/GameConfig.h"
#include "../logic/Model/GameEngine.h"
#include "../logic/Model/Position.h"
#include "../logic/Model/RealTimeArbiter.h"
#include "PieceRenderer.h"
#include "img.hpp"
#include <string>

/// ציור מלא של המסך: לוח + כלים + overlay מצב + סימון בחירה.
///
/// SRP: תפקיד יחיד — לתרגם את מצב ה-GameEngine + Arbiter לתמונה על המסך.
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
    void init(const GameEngine& engine);

    /// ציור מלא של פריים אחד על המסך
    /// @param screen buffer המסך
    /// @param engine מנוע המשחק
    /// @param arbiter מנהל התנועות (לאינטרפולציה)
    void render_frame(Img& screen, const GameEngine& engine,
                      const RealTimeArbiter& arbiter);

    /// גישה ל-PieceRenderer
    [[nodiscard]] PieceRenderer& piece_renderer() noexcept { return m_pieceRenderer; }

    /// קידום אנימציות ספרייט
    void tick_animations(int deltaMs) {
        m_pieceRenderer.advance_animations(deltaMs);
    }

private:
    void draw_motion_piece(Img& screen, const Motion& motion);
    void draw_waiting_overlay(Img& screen);
    void draw_gameover_overlay(Img& screen, const GameEngine& engine);
    void draw_selection_marker(Img& screen, Position pos, int cellW, int cellH);

    Img            m_boardImage;
    PieceRenderer  m_pieceRenderer;
    int m_cellWidth;
    int m_cellHeight;
};
