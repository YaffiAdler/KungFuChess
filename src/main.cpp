#include "Model/Board.h"
#include "Model/BoardParser.h"
#include "Model/CommandInterpreter.h"
#include "Model/GameEngine.h"
#include "Model/PixelMapper.h"
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

    // ── 3. יצירת GameEngine ו-PixelMapper ──
    GameConfig config;  // default: 8×8, 100px
    GameEngine engine(std::move(*boardOpt), config);
    PixelMapper pixelMapper(config);

    // ── 4. לולאת פקודות ──
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        auto parsed = CommandInterpreter::parse(line);

        switch (parsed.type) {

        case CommandType::Click: {
            // המרת פיקסלים ← PixelMapper (לא ב-GameEngine)
            auto cell = pixelMapper.to_cell(parsed.arg1, parsed.arg2,
                                            engine.board().rows(),
                                            engine.board().cols());
            if (!cell.has_value()) continue;  // מחוץ ללוח — התעלם

            if (!engine.selected().has_value()) {
                // אין כלי נבחר — נסה לבחור
                (void)engine.select(*cell);
            } else {
                // יש כלי נבחר — נסה להזיז
                auto result = engine.move_selected_to(*cell);
                if (!result.success && !result.message.empty()) {
                    std::cerr << result.message << '\n';
                }
                if (result.gameOver) {
                    // המשחק הסתיים
                }
            }
            break;
        }

        case CommandType::Wait:
            engine.advance_clock(std::chrono::milliseconds(parsed.arg1));
            break;

        case CommandType::PrintBoard: {
            // לוח עם סימון כלי נבחר
            auto sel = engine.selected();
            const Position* selPtr = sel.has_value() ? &(*sel) : nullptr;
            std::cout << engine.board().to_string(selPtr);
            break;
        }

        case CommandType::Unknown: {
            std::cerr << parsed.raw << '\n';  // Unknown command – pass through raw
            break;
        }
        }
    }

    return 0;
}
