#include "PieceRenderer.h"
#include <cctype>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

PieceRenderer::PieceRenderer(const std::string& piecesRootDir, int cellWidth, int cellHeight)
    : m_rootDir(piecesRootDir)
    , m_cellWidth(cellWidth)
    , m_cellHeight(cellHeight)
{}

std::string PieceRenderer::piece_code(const Piece& piece) {
    // הפורמט בתיקיות pieces1: {סוג}{צבע} — למשל RB = Rook Black, KW = King White
    // type_id: "rook", "king", "bishop", "queen", "knight", "pawn"
    const std::string& typeId = piece.type_id();
    char typeChar = static_cast<char>(std::toupper(static_cast<unsigned char>(typeId[0])));
    // "knight" → 'N' (כי K זה King)
    if (typeId == "knight") {
        typeChar = 'N';
    }
    char colorChar = (piece.get_color() == PieceColor::White) ? 'W' : 'B';
    return std::string(1, typeChar) + colorChar;
}

Img& PieceRenderer::get_or_load(const std::string& code) const {
    auto it = m_cache.find(code);
    if (it != m_cache.end()) {
        return it->second;
    }

    // נתיב: pieces1/{CODE}/states/idle/sprites/1.png
    fs::path piecesDir = fs::path(m_rootDir);
    fs::path codeDir = piecesDir / code;
    fs::path statesDir = codeDir / "states";
    fs::path idleDir = statesDir / "idle";
    fs::path spritesDir = idleDir / "sprites";
    fs::path spritePath = spritesDir / "1.png";

    std::string pathStr = spritePath.string();

    Img img;
    img.read(pathStr, {m_cellWidth, m_cellHeight}, true);

    m_cache[code] = std::move(img);
    return m_cache[code];
}

void PieceRenderer::set_cell_size(int width, int height) {
    m_cellWidth = width;
    m_cellHeight = height;
    // Clear cache so images reload with new size
    m_cache.clear();
}

void PieceRenderer::draw_piece(Img& screen, const Piece& piece) const {
    std::string code = piece_code(piece);
    Img& sprite = get_or_load(code);

    int x = piece.get_pos().col * m_cellWidth;
    int y = piece.get_pos().row * m_cellHeight;

    sprite.draw_on(screen, x, y);
}

void PieceRenderer::draw_all_pieces(Img& screen, const Board& board) const {
    for (const auto& cell : board.cells()) {
        if (cell.has_value()) {
            draw_piece(screen, *cell);
        }
    }
}
