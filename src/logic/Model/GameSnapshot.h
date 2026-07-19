#pragma once
#include "Board.h"
#include "Position.h"
#include "PieceColor.h"
#include "RealTimeArbiter.h"
#include <chrono>
#include <optional>
#include <vector>

/// תמונת מצב של המשחק — למנגנון שחזור/undo
struct GameSnapshot final {
    Board board;
    std::optional<Position> selectedPos;
    std::vector<Motion>     arbiterMotions;
    std::chrono::milliseconds gameClock{0};
    PieceColor currentTurn = PieceColor::White;
};
