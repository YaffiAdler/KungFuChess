#pragma once

#include "../logic/Model/Piece.h"
#include "../logic/Model/Board.h"
#include "img.hpp"
#include <string>
#include <unordered_map>

/// טעינה וציור של כלי שחמט על המסך.
/// משתמש במחלקת Img (מעטפת OpenCV) לטעינת sprites ולציור.
///
/// SRP: תפקיד יחיד — תרגום Piece → Sprite וציור על מסך.
/// אינו מכיר לוגיקת משחק, חוקים, או GameEngine.
class PieceRenderer final {
public:
    /// @param piecesRootDir  תיקיית השורש של pieces (למשל "src/graphics/pieces2")
    /// @param cellWidth      רוחב תא בפיקסלים
    /// @param cellHeight     גובה תא בפיקסלים
    PieceRenderer(const std::string& piecesRootDir, int cellWidth, int cellHeight);

    /// עדכון גודל תא — נקרא כשהלוח מוכן ויש לנו מידות מדויקות
    void set_cell_size(int width, int height);

    /// ציור כלי בודד על המסך
    void draw_piece(Img& screen, const Piece& piece) const;

    /// ציור כל הכלים מהלוח על המסך
    void draw_all_pieces(Img& screen, const Board& board) const;

private:
    /// המרת Piece לקוד תיקייה (למשל King White → "KW")
    static std::string piece_code(const Piece& piece);

    /// טעינה עצלה — טוען מהדיסק רק בפעם הראשונה
    Img& get_or_load(const std::string& code) const;

    mutable std::unordered_map<std::string, Img> m_cache;
    std::string m_rootDir;
    int m_cellWidth;
    int m_cellHeight;
};
