#include "Renderer.h"
#include <opencv2/imgproc.hpp>

namespace {

std::string board_image_path() { return "src/graphics/board.png"; }

} // anonymous

// ─────────────────────────────────────────────
//  Constructor
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
//  init — טעינת לוח + התאמת גדלי תאים
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
//  render_frame — ציור פריים שלם
// ─────────────────────────────────────────────
void Renderer::render_frame(Img& screen, const GameEngine& engine) {
    // 1. לוח רקע
    m_boardImage.draw_on(screen, 0, 0);

    // 2. כלים
    m_pieceRenderer.draw_all_pieces(screen, engine.board());

    // 3. Overlay מצב המתנה
    if (engine.state() == GameState::Waiting) {
        draw_waiting_overlay(screen);
    }

    // 4. סימון בחירה (רק ב-Playing)
    if (engine.state() == GameState::Playing) {
        auto sel = engine.selected();
        if (sel.has_value()) {
            draw_selection_marker(screen, *sel, m_cellWidth, m_cellHeight);
        }
    }
}

// ─────────────────────────────────────────────
//  draw_waiting_overlay
// ─────────────────────────────────────────────
void Renderer::draw_waiting_overlay(Img& screen) {
    cv::Mat overlay;
    screen.get_mat().copyTo(overlay);
    cv::addWeighted(overlay, 0.6, screen.get_mat(), 0.0, 0, screen.get_mat());

    int boardW = screen.get_mat().cols;
    int boardH = screen.get_mat().rows;
    std::string msg = "Press ENTER to start";
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(msg, cv::FONT_HERSHEY_DUPLEX, 1.5, 3, &baseline);
    cv::Point textOrg((boardW - textSize.width) / 2,
                      (boardH + textSize.height) / 2);
    cv::putText(screen.get_mat(), msg, textOrg,
                cv::FONT_HERSHEY_DUPLEX, 1.5,
                cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
}

// ─────────────────────────────────────────────
//  draw_selection_marker
// ─────────────────────────────────────────────
void Renderer::draw_selection_marker(Img& screen, Position pos,
                                     int cellW, int cellH) {
    int x = pos.col * cellW;
    int y = pos.row * cellH;
    cv::rectangle(screen.get_mat(),
                  cv::Rect(x, y, cellW, cellH),
                  cv::Scalar(0, 255, 0), 3);
}
