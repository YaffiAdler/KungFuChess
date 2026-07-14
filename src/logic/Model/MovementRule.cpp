#include "MovementRule.h"
#include <utility>

MovementRule MovementRule::step(std::vector<Direction> dirs) {
    return {MovePattern::Step, std::move(dirs), 1, true};
}

MovementRule MovementRule::slide(std::vector<Direction> dirs) {
    return {MovePattern::Slide, std::move(dirs), -1, true};
}

MovementRule MovementRule::jump(std::vector<Direction> offsets) {
    return {MovePattern::Jump, std::move(offsets), 1, true};
}
