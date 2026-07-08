#pragma once
#include "MovementRule.h"
#include <string>
#include <vector>

/// הגדרה של סוג כלי — DATA טהור, ללא קוד.
/// כל סוג כלי מוגדר על-ידי: מזהה, סימבול, וכללי תנועה.
/// בעתיד: ייטען ישירות מקובץ JSON/YAML.
struct PieceTypeDefinition final {
    std::string id;                         // "king", "queen", ...
    char        symbol = '?';               // 'K', 'Q', ...
    std::vector<MovementRule> rules;        // רשימת כללי תנועה (למשל רגלי: rule קדימה + rule מכות)
};
