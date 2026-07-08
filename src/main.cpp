#include "Model/Board.h"
#include "Model/BoardParser.h"
#include "Model/Game.h"
#include "Model/PieceTypeRegistry.h"
#include <iostream>
#include <string>

int main() {
    // ── 1. אתחול Registry — בעתיד יבוא מקובץ JSON ──
    PieceTypeRegistry::instance().register_standard();

    // ── 2. פענוח הלוח ──
    auto boardOpt = BoardParser::parse(std::cin, std::cerr);
    if (!boardOpt.has_value()) {
        return 1;
    }

    // ── 3. יצירת Game ──
    GameConfig config;  // default: 8×8, 100px
    Game game(std::move(*boardOpt), config);

    // ── 4. לולאת פקודות ──
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        CommandResult result = game.execute_command(line);

        if (!result.success) {
            // הודעות שגיאה ל-stderr
            std::cerr << result.message << '\n';
            continue;
        }

        if (!result.message.empty()) {
            std::cout << result.message;
            if (result.message.back() != '\n') {
                std::cout << '\n';
            }
        }

        if (result.exitRequested) {
            break;
        }
    }

    return 0;
}
