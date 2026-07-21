#pragma once

#include "../logic/Engine/GameSnapshot.h"
#include "PieceRenderer.h"
#include "img.hpp"
#include <string>

// ─────────────────────────────────────────────
// Renderer — מתזמן ציור: Board → Pieces → Animations → Overlay → Selection
// ─────────────────────────────────────────────
///
/// SRP: תפקיד יחיד — תרגום GameSnapshot לתמונה על המסך.
/// כל שלב בציור ממומש כמתודה פרטית; render_frame() משמש כ-pipeline ברמה גבוהה.
///
/// מקבל רק GameSnapshot — ללא Board, Piece, Motion, Arbiter.
class Renderer final {
public:
    /// @param boardImagePath נתיב לתמונת הלוח
    /// @param piecesRootDir  תיקיית sprites של הכלים
    /// @param cellWidth      רוחב ראשוני (יעודכן אוטומטית כשהלוח נטען)
    /// @param cellHeight     גובה ראשוני
    Renderer(const std::string& boardImagePath,
             const std::string& piecesRootDir,
             int cellWidth, int cellHeight);

    /// טוען תמונת לוח ומתאים גדלי תאים מה-snapshot
    void init(const GameSnapshot& snapshot);

    /// ציור מלא של פריים על המסך — pipeline
    void render_frame(Img& screen, const GameSnapshot& snapshot);

    /// קידום אנימציות — האצלה ל-PieceRenderer
    void tick_animations(int deltaMs) { m_pieceRenderer.advance_animations(deltaMs); }

    /// גישה ל-PieceRenderer (לשימוש חיצוני אם נחוץ)
    [[nodiscard]] PieceRenderer& piece_renderer() noexcept { return m_pieceRenderer; }

private:
    // ═══════════════════════════════════════════
    // שלבי ציור (pipeline)
    // ═══════════════════════════════════════════

    void draw_board(Img& screen);
    void draw_pieces(Img& screen, const GameSnapshot& snapshot);
    void draw_animations(Img& screen, const GameSnapshot& snapshot);
    void draw_state_overlay(Img& screen, const GameSnapshot& snapshot);
    void draw_selection(Img& screen, const GameSnapshot& snapshot);

    // ═══════════════════════════════════════════
    // Overlay helpers
    // ═══════════════════════════════════════════

    /// מעמעם את המסך כולו (darken)
    void apply_dim_overlay(Img& screen, double dimFactor);

    /// מעמעם את המסך ברקע כהה (alpha blending)
    void apply_dark_background(Img& screen, double opacity);

    /// Overlay של Waiting
    void draw_waiting_overlay(Img& screen);

    /// Overlay של GameOver
    void draw_gameover_overlay(Img& screen, const GameSnapshot& snapshot);

    // ═══════════════════════════════════════════
    // Text helpers
    // ═══════════════════════════════════════════

    /// מצייר טקסט במרכז המסך, בהיסט אנכי
    void draw_centered_text(Img& screen,
                            const std::string& text,
                            double fontSize,
                            int thickness,
                            const cv::Scalar& color,
                            int yOffset = 0,
                            cv::HersheyFonts fontFace = cv::FONT_HERSHEY_DUPLEX);

    /// מחזיר גודל טקסט (מבטל כפילות cv::getTextSize)
    [[nodiscard]] static cv::Size get_text_size(const std::string& text,
                                                 double fontSize,
                                                 int thickness,
                                                 cv::HersheyFonts fontFace = cv::FONT_HERSHEY_DUPLEX);

    // ═══════════════════════════════════════════
    // Coordinate helpers
    // ═══════════════════════════════════════════

    /// מרכז תא בפיקסלים
    [[nodiscard]] std::pair<double, double> cell_center(Position pos) const;

    /// המרת Position לפינת התא (top-left) בפיקסלים
    [[nodiscard]] std::pair<int, int> cell_top_left(Position pos) const;

    // ═══════════════════════════════════════════
    // Single-motion drawing
    // ═══════════════════════════════════════════

    void draw_motion_piece(Img& screen, const MotionInfo& motion);

    // ═══════════════════════════════════════════
    // Selection marker
    // ═══════════════════════════════════════════

    void draw_selection_marker(Img& screen, Position pos);

    // ═══════════════════════════════════════════
    // Public API constants (used externally)
    // ═══════════════════════════════════════════

public:
    static constexpr double kDimFactor        = 0.10;  // waiting overlay dim
    static constexpr double kGameOverOpacity  = 0.55;  // game-over dark background

    static constexpr double kTitleFontSize    = 2.5;
    static constexpr int    kTitleThickness   = 6;
    static constexpr int    kTitleYOffset     = -20;

    static constexpr double kWinnerFontSize   = 1.8;
    static constexpr int    kWinnerThickness  = 4;
    static constexpr int    kWinnerYOffset    = 60;

    static constexpr double kRestartFontSize  = 0.8;
    static constexpr int    kRestartThickness = 2;
    static constexpr int    kRestartYOffset   = 110;

    static constexpr double kPromptFontSize   = 1.5;
    static constexpr int    kPromptThickness  = 3;

    // Selection
    static constexpr int kSelectionThickness  = 3;

private:
    // ═══════════════════════════════════════════
    // Data
    // ═══════════════════════════════════════════

    Img           m_boardImage;
    PieceRenderer m_pieceRenderer;
    int           m_cellWidth;
    int           m_cellHeight;
};
