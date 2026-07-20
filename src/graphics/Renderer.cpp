#include "Renderer.h"
#include "../logic/Model/PieceColor.h"
#include "../logic/Model/MoveRecord.h"
#include <opencv2/imgproc.hpp>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace {

std::string board_image_path() { return "src/graphics/board.png"; }

/// פירמוט חותמת זמן: HH:MM:SS
std::string format_time(const std::chrono::system_clock::time_point& tp) {
    auto t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_buf;
    localtime_s(&tm_buf, &t);
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << tm_buf.tm_hour << ":"
        << std::setw(2) << tm_buf.tm_min  << ":"
        << std::setw(2) << tm_buf.tm_sec;
    return oss.str();
}

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
// init — טעינת לוח + התאמת גדלי תאים + פאנלים
// ─────────────────────────────────────────────
void Renderer::init(const GameSnapshot& snapshot, int panelWidth) {
    int boardW = m_boardImage.get_mat().cols;
    int boardH = m_boardImage.get_mat().rows;
    int rows = snapshot.board.rows();
    int cols = snapshot.board.cols();

    m_cellWidth  = boardW / cols;
    m_cellHeight = boardH / rows;
    m_panelWidth = panelWidth;
    m_pieceRenderer.set_cell_size(m_cellWidth, m_cellHeight);
    m_pieceRenderer.set_offset_x(m_panelWidth);
}

// ─────────────────────────────────────────────
// capture_timestamps — תיעוד חותמת זמן למהלכים חדשים
// ─────────────────────────────────────────────
void Renderer::capture_timestamps(const std::vector<MoveRecord>& history) {
    while (m_moveTimestamps.size() < history.size()) {
        m_moveTimestamps.push_back(format_time(std::chrono::system_clock::now()));
    }
}

// ─────────────────────────────────────────────
// render_frame — ציור פריים שלם (רב-תנועתי)
// ─────────────────────────────────────────────
void Renderer::render_frame(Img& screen, const GameSnapshot& snapshot,
                             const RealTimeArbiter& arbiter) {
    int boardW = m_boardImage.get_mat().cols;
    int boardH = m_boardImage.get_mat().rows;
    int totalW = boardW + 2 * m_panelWidth;

    // 0. צור canvas מורחב עם רקע כהה
    cv::Mat canvas(boardH, totalW, CV_8UC3, cv::Scalar(35, 35, 35));

    // העתקת תמונת הלוח למרכז (אופסט = m_panelWidth)
    cv::Mat boardRegion = canvas(cv::Rect(m_panelWidth, 0, boardW, boardH));
    m_boardImage.get_mat().copyTo(boardRegion);

    // העתקת ה-canvas ל-screen
    canvas.copyTo(screen.get_mat());

    // 1. בניית סט מיקומים לדילוג (כלים בתנועה — מקורות)
    std::unordered_set<Position> skipPositions;
    for (const auto& motion : arbiter.motions()) {
        skipPositions.insert(motion.from);
    }

    // 2. ציור כלים מהלוח (מדלג על תאי מקור של כלים בתנועה)
    m_pieceRenderer.draw_all_pieces(screen, snapshot.board, skipPositions);

    // 3. כל הכלים בתנועה — אינטרפולציה
    for (const auto& motion : arbiter.motions()) {
        draw_motion_piece(screen, motion);
    }

    // 4. Overlay מצב המתנה
    if (snapshot.state == GameState::Waiting) {
        draw_waiting_overlay(screen);
    }

    // 5. Overlay סיום משחק
    if (snapshot.state == GameState::GameOver) {
        draw_gameover_overlay(screen, snapshot);
    }

    // 6. סימון בחירה
    if (snapshot.state == GameState::Playing) {
        const auto& sel = snapshot.selectedPos;
        if (sel.has_value()) {
            draw_selection_marker(screen, *sel, m_cellWidth, m_cellHeight);
        }
    }

    // 7. תיעוד חותמות זמן וציור פאנלי היסטוריית מהלכים
    capture_timestamps(snapshot.moveHistory);
    draw_move_history(screen, snapshot.moveHistory);
}

// ─────────────────────────────────────────────
// draw_motion_piece — אינטרפולציה של כלי בודד בתנועה
// ─────────────────────────────────────────────
void Renderer::draw_motion_piece(Img& screen, const Motion& motion) {
    // חישוב התקדמות: elapsed / total, clamped [0, 1]
    double progress = (motion.totalMs > 0)
        ? static_cast<double>(motion.elapsedMs) / motion.totalMs
        : 1.0;
    if (progress > 1.0) progress = 1.0;

    double fromX = motion.from.col * m_cellWidth  + m_cellWidth  / 2.0 + m_panelWidth;
    double fromY = motion.from.row * m_cellHeight + m_cellHeight / 2.0;
    double toX   = motion.to.col   * m_cellWidth  + m_cellWidth  / 2.0 + m_panelWidth;
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

    // 2. מערבבים: 90% עוצמה של התמונה המקורית + 10% שקיפות (שחור)
    cv::addWeighted(original, 0.9,
                    cv::Mat::zeros(original.size(), original.type()), 0.1,
                    0, frame);

    // 3. טקסט במרכז
    std::string msg = "Press ENTER to start";
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(msg,
        cv::FONT_HERSHEY_DUPLEX, 1.5, 3, &baseline);
    cv::Point textOrg((frame.cols - textSize.width) / 2,
                      (frame.rows + textSize.height) / 2);
    cv::putText(frame, msg, textOrg,
                cv::FONT_HERSHEY_DUPLEX, 1.5,
                cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
}

// ─────────────────────────────────────────────
// draw_gameover_overlay
// ─────────────────────────────────────────────
void Renderer::draw_gameover_overlay(Img& screen, const GameSnapshot& snapshot) {
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
        cv::FONT_HERSHEY_DUPLEX, fontSize, thickness, &baseline);
    cv::Point textOrg((frame.cols - textSize.width) / 2,
                       frame.rows / 2 - 20);
    cv::putText(frame, gameOverText, textOrg,
                cv::FONT_HERSHEY_DUPLEX, fontSize,
                cv::Scalar(255, 255, 255), thickness, cv::LINE_AA);

    // Winner
    auto winner = snapshot.winner;
    if (winner.has_value()) {
        std::string winnerText = (*winner == PieceColor::White)
            ? "White Wins!" : "Black Wins!";
        cv::Size winSize = cv::getTextSize(winnerText.c_str(),
            cv::FONT_HERSHEY_DUPLEX, 1.8, 4, &baseline);
        cv::Point winOrg((frame.cols - winSize.width) / 2,
                          frame.rows / 2 + 60);
        cv::putText(frame, winnerText, winOrg,
                    cv::FONT_HERSHEY_DUPLEX, 1.8,
                    cv::Scalar(255, 215, 0), 4, cv::LINE_AA);
    }

    // Restart hint
    std::string restartHint = "Press ENTER to restart";
    cv::Size hintSize = cv::getTextSize(restartHint,
        cv::FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseline);
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
    int x = pos.col * cellW + m_panelWidth;
    int y = pos.row * cellH;
    cv::rectangle(screen.get_mat(),
                  cv::Rect(x, y, cellW, cellH),
                  cv::Scalar(0, 255, 0), 3);
}

// ─────────────────────────────────────────────
// draw_move_history — פאנלים צדדיים
// ─────────────────────────────────────────────
void Renderer::draw_move_history(Img& screen, const std::vector<MoveRecord>& history) {
    if (m_panelWidth <= 0) return;

    int boardW = m_boardImage.get_mat().cols;

    // פאנל שמאלי — מהלכי שחור
    draw_single_panel(screen, history, PieceColor::Black,
                      0, m_panelWidth);

    // פאנל ימני — מהלכי לבן
    draw_single_panel(screen, history, PieceColor::White,
                      m_panelWidth + boardW, m_panelWidth);
}

// ─────────────────────────────────────────────
// draw_single_panel — כתיבת פאנל בודד
// ─────────────────────────────────────────────
void Renderer::draw_single_panel(Img& screen,
                                  const std::vector<MoveRecord>& allMoves,
                                  PieceColor panelColor,
                                  int panelX, int panelWidth) {
    auto& frame = screen.get_mat();

    // סינון מהלכים לפי צבע
    std::vector<int> indices; // אינדקסים ב-allMoves השייכים לצבע זה
    for (size_t i = 0; i < allMoves.size(); ++i) {
        if (allMoves[i].player == panelColor) {
            indices.push_back(static_cast<int>(i));
        }
    }

    if (indices.empty()) return;

    int lineHeight = 20;
    int headerHeight = 30;
    int startY = 10;
    int paddingX = 6;

    // כותרת
    std::string header = (panelColor == PieceColor::White) ? "WHITE" : "BLACK";
    cv::Scalar headerColor = (panelColor == PieceColor::White)
        ? cv::Scalar(220, 220, 220)
        : cv::Scalar(180, 180, 180);
    cv::putText(frame, header,
                cv::Point(panelX + paddingX, startY + 18),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, headerColor, 2, cv::LINE_AA);

    // קו הפרדה מתחת לכותרת
    cv::line(frame,
             cv::Point(panelX + paddingX, startY + headerHeight),
             cv::Point(panelX + panelWidth - paddingX, startY + headerHeight),
             cv::Scalar(80, 80, 80), 1, cv::LINE_AA);

    // חישוב כמה מהלכים נכנסים
    int availableHeight = frame.rows - (startY + headerHeight + 8);
    int maxVisible = availableHeight / lineHeight;
    if (maxVisible < 1) maxVisible = 1;

    // הצג את המהלכים האחרונים בלבד
    int startIdx = std::max(0, static_cast<int>(indices.size()) - maxVisible);

    for (int i = startIdx; i < static_cast<int>(indices.size()); ++i) {
        int row = i - startIdx;
        int y = startY + headerHeight + 8 + row * lineHeight + 14;

        int globalIdx = indices[i];
        const MoveRecord& rec = allMoves[globalIdx];

        // תו אלגברי
        std::string notation = to_algebraic(rec);

        // חותמת זמן (אם קיימת)
        std::string displayLine = notation;
        if (globalIdx < static_cast<int>(m_moveTimestamps.size())) {
            displayLine += "  " + m_moveTimestamps[globalIdx];
        }

        // צבע טקסט: אפור בהיר
        cv::Scalar textColor = (panelColor == PieceColor::White)
            ? cv::Scalar(210, 210, 210)
            : cv::Scalar(170, 170, 170);

        cv::putText(frame, displayLine,
                    cv::Point(panelX + paddingX, y),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, textColor, 1, cv::LINE_AA);
    }
}
