#include "logic/Model/Board.h"
#include "logic/Model/BoardParser.h"
#include "logic/Model/GameEngine.h"
#include "logic/Model/GameConfig.h"
#include "logic/Model/PieceTypeRegistry.h"
#include "graphics/Window.h"
#include <iostream>

int main() {
    try {
        // ── 1. אתחול Registry — בעתיד יבוא מקובץ JSON ──
        PieceTypeRegistry::instance().register_standard();

        // ── 2. פענוח הלוח מתוך stdin ──
        auto boardOpt = BoardParser::parse(std::cin, std::cerr);
        if (!boardOpt.has_value()) {
            return 1;
        }

        // ── 3. יצירת GameEngine ──
        GameConfig config;  // default: 8×8, 100px
        GameEngine engine(std::move(*boardOpt), config);

        // ── 4. יצירת חלון גרפי והרצה ──
        Window window(config);
        window.set_engine(&engine);
        window.run();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
