#pragma once

#include "../logic/Model/Piece.h"
#include "../logic/Model/Board.h"
#include "../logic/Model/Position.h"
#include "img.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

/// מידע אנימציית Sprite לכל state של כלי.
struct SpriteAnimData final {
    std::vector<Img> sprites;       // 1.png, 2.png, ..., 5.png
    int     frames_per_sec  = 6;
    bool    is_loop         = true;
    int     frameCount      = 0;    // כמה sprites נטענו
    int     currentIndex    = 0;    // איזה sprite מוצג עכשיו
    int     msPerFrame      = 166;  // 1000 / frames_per_sec
    int     accumulatedMs   = 0;    // accumulative time for frame switching
};

/// טעינה וציור של כלי שחמט על המסך עם אנימציית Sprite.
/// SRP: תפקיד יחיד — תרגום Piece → Sprite (לפי PieceState) + אנימציה.
/// אינו מכיר לוגיקת משחק, חוקים, או GameEngine.
class PieceRenderer final {
public:
    PieceRenderer(const std::string& piecesRootDir, int cellWidth, int cellHeight);

    void set_cell_size(int width, int height);

    /// קידום אנימציות — מחליף sprite index לפי time elapsed
    void advance_animations(int deltaMs);

    /// ציור כלי בודד לפי מיקום לוגי
    void draw_piece(Img& screen, const Piece& piece) const;

    /// ציור כלי במיקום פיקסלים חופשי (עבור אינטרפולציה)
    void draw_piece_at(Img& screen, const Piece& piece, int x, int y) const;

    /// ציור כל הכלים מהלוח על המסך
    /// @param skipPos  אם מספקים, מדלג על התא הזה (כשכלי בתנועה — לא לצייר אותו בלוח)
    void draw_all_pieces(Img& screen, const Board& board,
                         std::optional<Position> skipPos = std::nullopt) const;

private:
    /// טוען (או מחזיר מ-cache) את אנימציית ה-sprites של code/state
    SpriteAnimData& get_or_load(const std::string& code,
                                 const std::string& stateName) const;

    mutable std::unordered_map<std::string, SpriteAnimData> m_cache;
    std::string m_rootDir;
    int m_cellWidth;
    int m_cellHeight;
};
