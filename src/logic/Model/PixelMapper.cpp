#include "PixelMapper.h"

std::optional<Position> PixelMapper::to_cell(int x, int y,
                                              int numRows, int numCols) const noexcept {
    if (x < 0 || y < 0) return std::nullopt;  // קואורדינטות שליליות — מחוץ ללוח

    int col = x / m_config->cellSizePixels;
    int row = y / m_config->cellSizePixels;

    if (row < 0 || row >= numRows || col < 0 || col >= numCols) {
        return std::nullopt;
    }

    return Position{row, col};
}
