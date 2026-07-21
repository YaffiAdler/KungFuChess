#include "Renderer.h"
#include <opencv2/imgproc.hpp>

namespace {

constexpr const char* kWaitingPrompt    = "Press ENTER to start";
constexpr const char* kGameOverTitle    = "GAME OVER";
constexpr const char* kWhiteWins        = "White Wins!";
constexpr const char* kBlackWins        = "Black Wins!";
constexpr const char* kRestartHint      = "Press ENTER to restart";

} // anonymous namespace

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
void Renderer::init(const GameSnapshot& snapshot) {
    int boardW = m_boardImage.get_mat().cols;
    int boardH = m_boardImage.get_mat().rows;
    int rows   = snapshot.boardRows;
    int cols   = snapshot.boardCols;

    // Validate — prevent division by zero
    if (rows <= 0 || cols <= 0) return;

    m_cellWidth  = boardW / cols;
    m_cellHeight = boardH / rows;
    m_pieceRenderer.set_cell_size(m_cellWidth, m_cellHeight);
}

// ═══════════════════════════════════════════════════
// render_frame — pipeline
// ═══════════════════════════════════════════════════
void Renderer::render_frame(Img& screen, const GameSnapshot& snapshot) {
    draw_board(screen);
    draw_pieces(screen, snapshot);
    draw_animations(screen, snapshot);
    draw_state_overlay(screen, snapshot);
    draw_selection(screen, snapshot);
}

// ─────────────────────────────────────────────
// draw_board — העתקת תמונת הלוח למסך
// ─────────────────────────────────────────────
void Renderer::draw_board(Img& screen) {
    m_boardImage.get_mat().copyTo(screen.get_mat());
}

// ─────────────────────────────────────────────
// draw_pieces — ציור כלים מהלוח
// ─────────────────────────────────────────────
void Renderer::draw_pieces(Img& screen, const GameSnapshot& snapshot) {
    m_pieceRenderer.draw_all_pieces(screen, snapshot);
}

// ─────────────────────────────────────────────
// draw_animations — ציור כלים בתנועה (אינטרפולציה)
// ─────────────────────────────────────────────
void Renderer::draw_animations(Img& screen, const GameSnapshot& snapshot) {
    for (const auto& motion : snapshot.motions) {
        draw_motion_piece(screen, motion);
    }
}

// ─────────────────────────────────────────────
// draw_state_overlay — Overlay מצב Waiting / GameOver
// ─────────────────────────────────────────────
void Renderer::draw_state_overlay(Img& screen, const GameSnapshot& snapshot) {
    if (snapshot.state == GameState::Waiting) {
        draw_waiting_overlay(screen);
    } else if (snapshot.state == GameState::GameOver) {
        draw_gameover_overlay(screen, snapshot);
    }
}

// ─────────────────────────────────────────────
// draw_selection — סימון תא נבחר
// ─────────────────────────────────────────────
void Renderer::draw_selection(Img& screen, const GameSnapshot& snapshot) {
    if (snapshot.state != GameState::Playing) return;

    const auto& sel = snapshot.selectedPos;
    if (sel.has_value()) {
        draw_selection_marker(screen, *sel);
    }
}

// ═══════════════════════════════════════════════════
// Coordinate helpers
// ═══════════════════════════════════════════════════

std::pair<double, double> Renderer::cell_center(Position pos) const {
    double cx = pos.col * m_cellWidth  + m_cellWidth  / 2.0;
    double cy = pos.row * m_cellHeight + m_cellHeight / 2.0;
    return {cx, cy};
}

std::pair<int, int> Renderer::cell_top_left(Position pos) const {
    int x = pos.col * m_cellWidth;
    int y = pos.row * m_cellHeight;
    return {x, y};
}

// ═══════════════════════════════════════════════════
// draw_motion_piece — אינטרפולציה מ-MotionInfo (DTO)
// ═══════════════════════════════════════════════════

void Renderer::draw_motion_piece(Img& screen, const MotionInfo& motion) {
    auto [fromCx, fromCy] = cell_center(motion.from);
    auto [toCx,   toCy]   = cell_center(motion.to);

    double progress = motion.progress;

    int drawX = static_cast<int>(fromCx + (toCx - fromCx) * progress - m_cellWidth  / 2.0);
    int drawY = static_cast<int>(fromCy + (toCy - fromCy) * progress - m_cellHeight / 2.0);

    m_pieceRenderer.draw_piece_at(screen, motion.piece, drawX, drawY);
}

// ═══════════════════════════════════════════════════
// Overlay helpers
// ═══════════════════════════════════════════════════

void Renderer::apply_dim_overlay(Img& screen, double dimFactor) {
    cv::Mat& frame = screen.get_mat();
    cv::Mat original;
    frame.copyTo(original);

    cv::addWeighted(original, 1.0 - dimFactor,
                    cv::Mat::zeros(original.size(), original.type()), dimFactor,
                    0, frame);
}

void Renderer::apply_dark_background(Img& screen, double opacity) {
    cv::Mat& frame = screen.get_mat();

    cv::Mat overlay;
    frame.copyTo(overlay);
    cv::rectangle(overlay,
                  cv::Point(0, 0),
                  cv::Point(frame.cols, frame.rows),
                  cv::Scalar(0, 0, 0), cv::FILLED);

    cv::addWeighted(overlay, opacity, frame, 1.0 - opacity, 0, frame);
}

// ─────────────────────────────────────────────
// draw_waiting_overlay
// ─────────────────────────────────────────────
void Renderer::draw_waiting_overlay(Img& screen) {
    apply_dim_overlay(screen, kDimFactor);
    draw_centered_text(screen, kWaitingPrompt,
                       kPromptFontSize, kPromptThickness,
                       cv::Scalar(255, 255, 255));
}

// ─────────────────────────────────────────────
// draw_gameover_overlay
// ─────────────────────────────────────────────
void Renderer::draw_gameover_overlay(Img& screen, const GameSnapshot& snapshot) {
    apply_dark_background(screen, kGameOverOpacity);

    // Title
    draw_centered_text(screen, kGameOverTitle,
                       kTitleFontSize, kTitleThickness,
                       cv::Scalar(255, 255, 255), kTitleYOffset);

    // Winner
    const auto& winner = snapshot.winner;
    if (winner.has_value()) {
        const char* winnerText = (*winner == PieceColor::White) ? kWhiteWins : kBlackWins;
        draw_centered_text(screen, winnerText,
                           kWinnerFontSize, kWinnerThickness,
                           cv::Scalar(255, 215, 0), kWinnerYOffset);
    }

    // Restart hint — uses SIMPLEX (matching original)
    draw_centered_text(screen, kRestartHint,
                       kRestartFontSize, kRestartThickness,
                       cv::Scalar(200, 200, 200), kRestartYOffset,
                       cv::FONT_HERSHEY_SIMPLEX);
}

// ═══════════════════════════════════════════════════
// Text helpers
// ═══════════════════════════════════════════════════

cv::Size Renderer::get_text_size(const std::string& text,
                                  double fontSize, int thickness,
                                  cv::HersheyFonts fontFace) {
    int baseline = 0;
    return cv::getTextSize(text, fontFace,
                           fontSize, thickness, &baseline);
}

void Renderer::draw_centered_text(Img& screen,
                                   const std::string& text,
                                   double fontSize,
                                   int thickness,
                                   const cv::Scalar& color,
                                   int yOffset,
                                   cv::HersheyFonts fontFace) {
    cv::Mat& frame = screen.get_mat();
    cv::Size textSize = get_text_size(text, fontSize, thickness, fontFace);

    int x = (frame.cols - textSize.width) / 2;
    int y = (frame.rows + textSize.height) / 2 + yOffset;

    cv::putText(frame, text, cv::Point(x, y),
                fontFace, fontSize,
                color, thickness, cv::LINE_AA);
}

// ═══════════════════════════════════════════════════
// Selection marker
// ═══════════════════════════════════════════════════

void Renderer::draw_selection_marker(Img& screen, Position pos) {
    auto [x, y] = cell_top_left(pos);
    cv::rectangle(screen.get_mat(),
                  cv::Rect(x, y, m_cellWidth, m_cellHeight),
                  cv::Scalar(255, 255, 0), kSelectionThickness);
}
