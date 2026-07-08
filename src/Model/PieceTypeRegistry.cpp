#include "PieceTypeRegistry.h"
#include <stdexcept>

PieceTypeRegistry& PieceTypeRegistry::instance() {
    static PieceTypeRegistry inst;
    return inst;
}

void PieceTypeRegistry::register_type(PieceTypeDefinition def) {
    auto id = def.id;  // copy before move
    auto sym = def.symbol;
    auto ptr = std::make_unique<PieceTypeDefinition>(std::move(def));
    m_bySymbol[sym] = ptr.get();
    m_byId[id] = std::move(ptr);
}

const PieceTypeDefinition* PieceTypeRegistry::find_by_id(const std::string& id) const {
    auto it = m_byId.find(id);
    return (it != m_byId.end()) ? it->second.get() : nullptr;
}

const PieceTypeDefinition* PieceTypeRegistry::find_by_symbol(char symbol) const {
    auto it = m_bySymbol.find(symbol);
    return (it != m_bySymbol.end()) ? it->second : nullptr;
}

void PieceTypeRegistry::clear() {
    m_byId.clear();
    m_bySymbol.clear();
}

// ─────────────────────────────────────────────
//  register_standard — DATA, לא CODE.
//  6 כלי השחמט הסטנדרטיים, כל אחד עם MovementRules.
// ─────────────────────────────────────────────
void PieceTypeRegistry::register_standard() {
    using enum MovePattern;

    // King — צעד אחד ב-8 כיוונים
    register_type(PieceTypeDefinition{
        "king", 'K',
        { MovementRule::step({
            {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}
        })}
    });

    // Queen — החלקה ב-8 כיוונים
    register_type(PieceTypeDefinition{
        "queen", 'Q',
        { MovementRule::slide({
            {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}
        })}
    });

    // Rook — החלקה ב-4 כיוונים קרדינליים
    register_type(PieceTypeDefinition{
        "rook", 'R',
        { MovementRule::slide({
            {-1,0},{1,0},{0,-1},{0,1}
        })}
    });

    // Bishop — החלקה ב-4 אלכסונים
    register_type(PieceTypeDefinition{
        "bishop", 'B',
        { MovementRule::slide({
            {-1,-1},{-1,1},{1,-1},{1,1}
        })}
    });

    // Knight — קפיצות L
    register_type(PieceTypeDefinition{
        "knight", 'N',
        { MovementRule::jump({
            {-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}
        })}
    });

    // Pawn — צעד קדימה (maxSteps=2 מאפשר שני צעדים בהתחלה)
    register_type(PieceTypeDefinition{
        "pawn", 'P',
        { MovementRule::step({
            {-1,0}   // קדימה — הכיוון בפועל נקבע לפי צבע ב-MoveGenerator
        })}
    });
    // שימו לב: maxSteps=2 (ברירת מחדל) → MoveGenerator יוסיף צעד שני רק לרגלי שלא זז
}
