#include "Renderer.h"
#include "../logic/Model/PieceColor.h"
#include <opencv2/imgproc.hpp>

namespace {

std::string board_image_path() { return "src/graphics/board.png"; }

} // anonymous

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
Renderer::Renderer(const std::string& boardImagePath,
                   const std::string& piecesRootDir,
                   int cellWidth, int cellHeight)
    : m_pieceRenderer(piecesRootDir, cellWidth, cellHeight)
    , m_cellWidth(cellWidth)
    , m_cellHeight(cellHeight)
{
    m_boardImage.read(boardImagePath);
}

// ─────────────────────────────────────────────
// init — טעינת לוח + התאמת גדלי תאים
// ─────────────────────────────────────────────
void Renderer::init(const GameEngine& engine) {
    int boardW = m_boardImage.get_mat().cols;
    int boardH = m_boardImage.get_mat().rows;
    int rows   = engine.board().rows();
    int cols   = engine.board().cols();

    m_cellWidth  = boardW / cols;
    m_cellHeight = boardH / rows;
    m_pieceRenderer.set_cell_size(m_cellWidth, m_cellHeight);
}

// ─────────────────────────────────────────────
// render_frame — ציור פריים שלם
// ─────────────────────────────────────────────
void Renderer::render_frame(Img& screen, const GameEngine& engine,
                             const RealTimeArbiter& arbiter) {
    // 0. ניקוי מסך — העתקת תמונת הלוח (מוחק ציור קודם)
    m_boardImage.get_mat().copyTo(screen.get_mat());

    // 1. כלים מהלוח הלוגי (מדלג על תא המקור — הכלי בתנועה)
    std::optional<Position> skipPos;
    if (arbiter.hasActiveMotion()) {
        skipPos = arbiter.motion()->from;
    }
    m_pieceRenderer.draw_all_pieces(screen, engine.board(), skipPos);

    // 2. כלי בתנועה — אינטרפולציה
    if (arbiter.hasActiveMotion()) {
        draw_motion_piece(screen, *arbiter.motion());
    }

    // 3. Overlay מצב המתנה
    if (engine.state() == GameState::Waiting) {
        draw_waiting_overlay(screen);
    }

    // 4. Overlay סיום משחק
    if (engine.state() == GameState::GameOver) {
        draw_gameover_overlay(screen, engine);
    }

    // 5. סימון בחירה
    if (engine.state() == GameState::Playing) {
        auto sel = engine.selected();
        if (sel.has_value()) {
            draw_selection_marker(screen, *sel, m_cellWidth, m_cellHeight);
        }
    }
}

// ─────────────────────────────────────────────
// draw_motion_piece — אינטרפולציה גרפית
// ─────────────────────────────────────────────
void Renderer::draw_motion_piece(Img& screen, const Motion& motion) {
    // חישוב progress: 0.0 (התחלה) עד 1.0 (סיום)
    double progress = static_cast<double>(motion.elapsedMs) / motion.totalMs;
    if (progress > 1.0) progress = 1.0;

    // מיקום אינטרפולציה בפיקסלים
    double fromX = motion.from.col * m_cellWidth  + m_cellWidth  / 2.0;
    double fromY = motion.from.row * m_cellHeight + m_cellHeight / 2.0;
    double toX   = motion.to.col   * m_cellWidth  + m_cellWidth  / 2.0;
    double toY   = motion.to.row   * m_cellHeight + m_cellHeight / 2.0;

    int drawX = static_cast<int>(fromX + (toX - fromX) * progress - m_cellWidth  / 2.0);
    int drawY = static_cast<int>(fromY + (toY - fromY) * progress - m_cellHeight / 2.0);

    // ציור הכלי במיקום האינטרפולציה
    m_pieceRenderer.draw_piece_at(screen, motion.piece, drawX, drawY);
}

// ─────────────────────────────────────────────
// draw_waiting_overlay
// ─────────────────────────────────────────────
void Renderer::draw_waiting_overlay(Img& screen) {
    cv::Mat& frame = screen.get_mat();

    // 1. יוצרים עותק זמני של הפריים הנוכחי
    cv::Mat original;
    frame.copyTo(original);

    // 2. מערבבים: 60% עוצמה של התמונה המקורית + 40% שקיפות (שחור)
    cv::addWeighted(original, 0.6,
                    cv::Mat::zeros(original.size(), original.type()), 0.4,
                    0, frame);

    // 3. טקסט במרכז
    std::string msg = "Press ENTER to start";
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(msg, cv::FONT_HERSHEY_DUPLEX, 1.5, 3, &baseline);
    cv::Point textOrg((frame.cols - textSize.width) / 2,
                      (frame.rows + textSize.height) / 2);
    cv::putText(frame, msg, textOrg,
                cv::FONT_HERSHEY_DUPLEX, 1.5,
                cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
}

// ─────────────────────────────────────────────
// draw_gameover_overlay
// ─────────────────────────────────────────────
void Renderer::draw_gameover_overlay(Img& screen, const GameEngine& engine) {
    auto& frame = screen.get_mat();

    // רקע חצי-שקוף
    cv::Mat overlay;
    frame.copyTo(overlay);
    cv::rectangle(overlay,
                  cv::Point(0, 0),
                  cv::Point(frame.cols, frame.rows),
                  cv::Scalar(0, 0, 0), cv::FILLED);
    cv::addWeighted(overlay, 0.55, frame, 0.45, 0, frame);

    // כותרת Game Over
    std::string gameOverText = "GAME OVER";
    double fontSize = 2.5;
    int thickness = 6;
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(gameOverText,
                                         cv::FONT_HERSHEY_DUPLEX,
                                         fontSize, thickness, &baseline);
    cv::Point textOrg((frame.cols - textSize.width) / 2,
                      frame.rows / 2 - 20);
    cv::putText(frame, gameOverText, textOrg,
                cv::FONT_HERSHEY_DUPLEX, fontSize,
                cv::Scalar(255, 255, 255), thickness, cv::LINE_AA);

    // Winner
    auto winner = engine.winner();
    if (winner.has_value()) {
        std::string winnerText = (*winner == PieceColor::White)
                                    ? "White Wins!"
                                    : "Black Wins!";
        cv::Size winSize = cv::getTextSize(winnerText.c_str(),
                                            cv::FONT_HERSHEY_DUPLEX,
                                            1.8, 4, &baseline);
        cv::Point winOrg((frame.cols - winSize.width) / 2,
                         frame.rows / 2 + 60);
        cv::putText(frame, winnerText, winOrg,
                    cv::FONT_HERSHEY_DUPLEX, 1.8,
                    cv::Scalar(255, 215, 0), 4, cv::LINE_AA);
    }

    // Restart hint
    std::string restartHint = "Press ENTER to restart";
    cv::Size hintSize = cv::getTextSize(restartHint,
                                         cv::FONT_HERSHEY_SIMPLEX,
                                         0.8, 2, &baseline);
    cv::Point hintOrg((frame.cols - hintSize.width) / 2,
                      frame.rows / 2 + 110);
    cv::putText(frame, restartHint, hintOrg,
                cv::FONT_HERSHEY_SIMPLEX, 0.8,
                cv::Scalar(200, 200, 200), 2, cv::LINE_AA);
}

// ─────────────────────────────────────────────
// draw_selection_marker
// ─────────────────────────────────────────────
void Renderer::draw_selection_marker(Img& screen, Position pos,
                                      int cellW, int cellH) {
    int x = pos.col * cellW;
    int y = pos.row * cellH;
    cv::rectangle(screen.get_mat(),
                  cv::Rect(x, y, cellW, cellH),
                  cv::Scalar(0, 255, 0), 3);
}
