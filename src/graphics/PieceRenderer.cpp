#include "PieceRenderer.h"
#include "SpriteAnimConfig.h"
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
PieceRenderer::PieceRenderer(const std::string& piecesRootDir,
                              int cellWidth, int cellHeight)
    : m_rootDir(piecesRootDir)
    , m_cellWidth(cellWidth)
    , m_cellHeight(cellHeight)
{}

// ─────────────────────────────────────────────
// anim_key — "row,col/stateName"
// ─────────────────────────────────────────────
std::string PieceRenderer::anim_key(Position pos, const std::string& stateName) {
    return std::to_string(pos.row) + "," + std::to_string(pos.col)
           + "/" + stateName;
}

// ─────────────────────────────────────────────
// get_frames — load shared sprite frames (cached)
// ─────────────────────────────────────────────
SpriteAnimFrames& PieceRenderer::get_frames(
    const std::string& code, const std::string& stateName)
{
    std::string key = code + "/" + stateName;
    auto it = m_framesCache.find(key);
    if (it != m_framesCache.end()) return it->second;

    SpriteAnimFrames frames;

    // Load config.json (graphics section)
    SpriteAnimConfig cfg = load_sprite_anim_config(m_rootDir, code, stateName);
    frames.msPerFrame = (cfg.frames_per_sec > 0) ? 1000 / cfg.frames_per_sec : 166;
    frames.is_loop = cfg.is_loop;

    // Load 1.png, 2.png, ... until missing
    std::string stateDir = m_rootDir + "/" + code + "/states/" + stateName + "/sprites";
    for (int i = 1; ; ++i) {
        fs::path spritePath = fs::path(stateDir) / (std::to_string(i) + ".png");
        if (!fs::exists(spritePath)) break;
        Img img;
        img.read(spritePath.string(), {m_cellWidth, m_cellHeight}, true);
        frames.sprites.push_back(std::move(img));
    }

    frames.frameCount = static_cast<int>(frames.sprites.size());

    // Fallback: if no sprites loaded, try idle/1.png
    if (frames.frameCount == 0 && stateName != "idle") {
        std::string fallbackDir = m_rootDir + "/" + code + "/states/idle";
        fs::path fallback = fs::path(fallbackDir) / "1.png";
        if (fs::exists(fallback)) {
            Img img;
            img.read(fallback.string(), {m_cellWidth, m_cellHeight}, true);
            frames.sprites.push_back(std::move(img));
            frames.frameCount = 1;
        }
    }

    auto result = m_framesCache.emplace(key, std::move(frames));
    return result.first->second;
}

// ─────────────────────────────────────────────
// get_anim_state — per-instance AnimState
// ─────────────────────────────────────────────
AnimState& PieceRenderer::get_anim_state(Position pos,
                                          const std::string& stateName,
                                          const SpriteAnimFrames& frames)
{
    std::string key = anim_key(pos, stateName);
    auto it = m_animStates.find(key);
    if (it != m_animStates.end()) return it->second;

    AnimState state;
    state.msPerFrame = frames.msPerFrame;
    state.frameCount = frames.frameCount;
    state.is_loop = frames.is_loop;
    state.currentIndex = 0;

    auto result = m_animStates.emplace(key, std::move(state));
    return result.first->second;
}

// ─────────────────────────────────────────────
// advance_animations — per-cell frame advance
// ─────────────────────────────────────────────
void PieceRenderer::advance_animations(int deltaMs) {
    for (auto& [key, state] : m_animStates) {
        if (state.msPerFrame <= 0) continue;
        if (state.frameCount <= 0) continue;

        state.accumulatedMs += deltaMs;
        while (state.accumulatedMs >= state.msPerFrame) {
            state.accumulatedMs -= state.msPerFrame;
            if (state.currentIndex + 1 < state.frameCount) {
                state.currentIndex++;
            } else if (state.is_loop) {
                state.currentIndex = 0;
            }
            // non-loop: stay on last frame
        }
    }
}

// ─────────────────────────────────────────────
// set_cell_size — clear caches, reload at new size
// ─────────────────────────────────────────────
void PieceRenderer::set_cell_size(int width, int height) {
    m_cellWidth = width;
    m_cellHeight = height;
    m_framesCache.clear();
    m_animStates.clear();
}

// ─────────────────────────────────────────────
// draw_piece — draw using shared frames + per-instance index
// ─────────────────────────────────────────────
void PieceRenderer::draw_piece(Img& screen, const Piece& piece) {
    std::string code = piece.get_code();
    std::string stateName = state_to_string(piece.get_state());

    SpriteAnimFrames& frames = get_frames(code, stateName);
    if (frames.sprites.empty()) return;

    AnimState& anim = get_anim_state(piece.get_pos(), stateName, frames);

    // Clamp index in case frameCount changed (shouldn't, but safe)
    int idx = anim.currentIndex;
    if (idx >= frames.frameCount) idx = frames.frameCount - 1;

    int x = piece.get_pos().col * m_cellWidth + m_offsetX;
    int y = piece.get_pos().row * m_cellHeight;

    frames.sprites[idx].draw_on(screen, x, y);
}

// ─────────────────────────────────────────────
// draw_piece_at — draw at arbitrary pixel coords (interpolation)
// ─────────────────────────────────────────────
void PieceRenderer::draw_piece_at(Img& screen, const Piece& piece,
                                   int x, int y) {
    std::string code = piece.get_code();
    std::string stateName = state_to_string(piece.get_state());

    SpriteAnimFrames& frames = get_frames(code, stateName);
    if (frames.sprites.empty()) return;

    AnimState& anim = get_anim_state(piece.get_pos(), stateName, frames);

    int idx = anim.currentIndex;
    if (idx >= frames.frameCount) idx = frames.frameCount - 1;

    frames.sprites[idx].draw_on(screen, x, y);
}

// ─────────────────────────────────────────────
// draw_all_pieces — all board pieces, skipping positions in set
// ─────────────────────────────────────────────
void PieceRenderer::draw_all_pieces(Img& screen, const Board& board,
                                     const std::unordered_set<Position>& skipPositions) {
    for (int r = 0; r < board.rows(); ++r) {
        for (int c = 0; c < board.cols(); ++c) {
            if (skipPositions.count(Position{r, c}) > 0) continue;

            const auto& cell = board.at(r, c);
            if (cell.has_value()) {
                draw_piece(screen, *cell);
            }
        }
    }
}
