#include "PieceRenderer.h"
#include "../logic/Model/PieceStateConfig.h"
#include <cctype>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

PieceRenderer::PieceRenderer(const std::string& piecesRootDir, int cellWidth, int cellHeight)
    : m_rootDir(piecesRootDir)
    , m_cellWidth(cellWidth)
    , m_cellHeight(cellHeight)
{}

// ─────────────────────────────────────────────
// advance_animations — קידום sprite index
// ─────────────────────────────────────────────
void PieceRenderer::advance_animations(int deltaMs) {
    for (auto& [key, anim] : m_cache) {
        if (anim.sprites.empty()) continue;
        if (anim.msPerFrame <= 0)  continue;

        anim.accumulatedMs += deltaMs;

        while (anim.accumulatedMs >= anim.msPerFrame) {
            anim.accumulatedMs -= anim.msPerFrame;

            if (anim.currentIndex + 1 < anim.frameCount) {
                // יש sprite הבא
                anim.currentIndex++;
            } else if (anim.is_loop) {
                // הגענו לסוף + is_loop → חוזרים להתחלה
                anim.currentIndex = 0;
            }
            // else: הגענו לסוף + !is_loop → נשארים ב-currentIndex
        }
    }
}

// ─────────────────────────────────────────────
// get_or_load — טוען sprites לפי code + stateName
// ─────────────────────────────────────────────
SpriteAnimData& PieceRenderer::get_or_load(
    const std::string& code, const std::string& stateName) const
{
    std::string key = code + "/" + stateName;
    auto it = m_cache.find(key);
    if (it != m_cache.end()) {
        return it->second;
    }

    SpriteAnimData data;

    // טעינת config.json — גרפיקה בלבד (מחלץ את מפתח "graphics")
    SpriteAnimConfig cfg = load_sprite_anim_config(m_rootDir, code, stateName);
    data.frames_per_sec = cfg.frames_per_sec;
    data.is_loop        = cfg.is_loop;
    data.msPerFrame     = (data.frames_per_sec > 0) ? 1000 / data.frames_per_sec : 166;
    data.currentIndex   = 0;
    data.accumulatedMs  = 0;

    // טעינת sprites: 1.png, 2.png, ... עד שאין יותר
    fs::path basePath = fs::path(m_rootDir) / code / "states" / stateName / "sprites";
    for (int i = 1; ; ++i) {
        fs::path spritePath = basePath / (std::to_string(i) + ".png");
        if (!fs::exists(spritePath)) break;

        Img img;
        img.read(spritePath.string(), {m_cellWidth, m_cellHeight}, true);
        data.sprites.push_back(std::move(img));
    }

    data.frameCount = static_cast<int>(data.sprites.size());
    if (data.frameCount == 0) {
        // fallback: טוען idle/1.png
        fs::path fallback = fs::path(m_rootDir) / code / "states" / "idle" / "sprites" / "1.png";
        Img img;
        img.read(fallback.string(), {m_cellWidth, m_cellHeight}, true);
        data.sprites.push_back(std::move(img));
        data.frameCount = 1;
    }

    auto result = m_cache.emplace(key, std::move(data));
    return result.first->second;
}

// ─────────────────────────────────────────────
// set_cell_size — מנקה cache, טוען מחדש בגודל חדש
// ─────────────────────────────────────────────
void PieceRenderer::set_cell_size(int width, int height) {
    m_cellWidth = width;
    m_cellHeight = height;
    m_cache.clear();
}

// ─────────────────────────────────────────────
// draw_piece
// ─────────────────────────────────────────────
void PieceRenderer::draw_piece(Img& screen, const Piece& piece) const {
    std::string code = piece.get_code();
    std::string stateName = state_to_string(piece.get_state());

    SpriteAnimData& animData = get_or_load(code, stateName);
    if (animData.sprites.empty()) return;

    Img& sprite = animData.sprites[animData.currentIndex];
    int x = piece.get_pos().col * m_cellWidth;
    int y = piece.get_pos().row * m_cellHeight;

    sprite.draw_on(screen, x, y);
}

// ─────────────────────────────────────────────
// draw_piece_at
// ─────────────────────────────────────────────
void PieceRenderer::draw_piece_at(Img& screen, const Piece& piece, int x, int y) const {
    std::string code = piece.get_code();
    std::string stateName = state_to_string(piece.get_state());

    SpriteAnimData& animData = get_or_load(code, stateName);
    if (animData.sprites.empty()) return;

    Img& sprite = animData.sprites[animData.currentIndex];
    sprite.draw_on(screen, x, y);
}

// ─────────────────────────────────────────────
// draw_all_pieces — עם skipPos אופציונלי
// ─────────────────────────────────────────────
void PieceRenderer::draw_all_pieces(
    Img& screen, const Board& board,
    std::optional<Position> skipPos) const
{
    for (int r = 0; r < board.rows(); ++r) {
        for (int c = 0; c < board.cols(); ++c) {
            if (skipPos.has_value() &&
                skipPos->row == r && skipPos->col == c) {
                continue; // מדלג על תא המקור (הכלי בתנועה)
            }
            const auto& cell = board.at(r, c);
            if (cell.has_value()) {
                draw_piece(screen, *cell);
            }
        }
    }
}
