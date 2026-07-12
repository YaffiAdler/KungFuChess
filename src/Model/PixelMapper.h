#pragma once
#include "Position.h"
#include "GameConfig.h"
#include <optional>

/// המרת קואורדינטות פיקסלים ← תא לוח.
/// SRP: תפקיד יחיד — המרה גאומטרית, ללא ידע על המשחק.
class PixelMapper final {
public:
    explicit PixelMapper(const GameConfig& config) noexcept : m_config(&config) {}

    /// ממיר קואורדינטות פיקסלים (x,y) לתא לוח.
    /// מחזיר std::nullopt אם הקואורדינטות מחוץ לגבולות הלוח.
    [[nodiscard]] std::optional<Position> to_cell(int x, int y,
                                                   int numRows, int numCols) const noexcept;

private:
    const GameConfig* m_config;
};
