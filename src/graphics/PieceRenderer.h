#pragma once
#include "../logic/Model/Piece.h"
#include "../logic/Model/Board.h"
#include "../logic/Model/Position.h"
#include "img.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>

/// Sprite frames shared across all pieces of same type+state.
/// Loaded once per (code, stateName), never mutated after loading.
struct SpriteAnimFrames final {
    std::vector<Img> sprites;
    int frameCount = 0;
    int msPerFrame = 166; // 1000 / frames_per_sec
    bool is_loop = true;
};

/// Per-instance animation — one per (position, stateName) pair.
/// Each piece tracks its own current frame independently.
struct AnimState final {
    int currentIndex = 0;
    int accumulatedMs = 0;
    int msPerFrame = 166;
    int frameCount = 0;
    bool is_loop = true;
};

/// טעינה וציור של כלי שחמט על המסך עם אנימציית Sprite.
/// Sprite frames SHARED across all pieces of same type+state.
/// Animation index is PER CELL+STATE — each piece runs independently.
class PieceRenderer final {
public:
    PieceRenderer(const std::string& piecesRootDir, int cellWidth, int cellHeight);

    void set_cell_size(int width, int height);
    void set_offset_x(int offsetX) noexcept { m_offsetX = offsetX; }

    /// קידום אנימציות — מחליף sprite index לפי time elapsed
    void advance_animations(int deltaMs);

    /// ציור כלי בודד לפי מיקום לוגי
    void draw_piece(Img& screen, const Piece& piece);

    /// ציור כלי במיקום פיקסלים חופשי (עבור אינטרפולציה)
    void draw_piece_at(Img& screen, const Piece& piece, int x, int y);

    /// ציור כל הכלים מהלוח על המסך
    /// @param skipPositions סט מיקומים לדלג עליהם (כלים בתנועה — יצוירו ע"י draw_motion_piece)
    void draw_all_pieces(Img& screen, const Board& board,
                         const std::unordered_set<Position>& skipPositions);

private:
    /// מפתח אנימציה: "row,col/stateName"
    static std::string anim_key(Position pos, const std::string& stateName);

    /// Lazy-load shared SpriteAnimFrames (cached)
    SpriteAnimFrames& get_frames(const std::string& code,
                                  const std::string& stateName);

    /// Get-or-create per-instance AnimState
    AnimState& get_anim_state(Position pos,
                               const std::string& stateName,
                               const SpriteAnimFrames& frames);

    // Cache: shared sprite frames (key = "code/stateName")
    mutable std::unordered_map<std::string, SpriteAnimFrames> m_framesCache;
    // Per-instance animation state (key = "row,col/stateName")
    mutable std::unordered_map<std::string, AnimState> m_animStates;

    std::string m_rootDir;
    int m_cellWidth;
    int m_cellHeight;
    int m_offsetX = 0;
};
