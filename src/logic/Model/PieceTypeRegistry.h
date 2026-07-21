#pragma once
#include "PieceTypeDefinition.h"
#include "PieceTypeRegistry.h"
#include "../Rules/MovementRule.h"
#include <unordered_map>
#include <string>
#include <memory>

/// Singleton — מרכז כל סוגי הכלים הזמינים במשחק.
///
/// SRP: תפקיד יחיד — לנהל את מאגר הגדרות הכלים.
/// בעתיד: ייטען מקובץ JSON/YAML במקום register_standard().
class PieceTypeRegistry final {
public:
    /// גישה ל-Singleton
    static PieceTypeRegistry& instance();

    /// רישום סוג כלי חדש למאגר
    void register_type(PieceTypeDefinition def);

    /// חיפוש לפי מזהה (למשל "king")
    [[nodiscard]] const PieceTypeDefinition* find_by_id(const std::string& id) const;

    /// חיפוש לפי סימבול (למשל 'K')
    [[nodiscard]] const PieceTypeDefinition* find_by_symbol(char symbol) const;

    /// ניקוי המאגר (לשימוש בבדיקות)
    void clear();

    /// רישום 6 הכלים הסטנדרטיים של שחמט — נקרא פעם אחת באתחול
    void register_standard();

private:
    PieceTypeRegistry() = default;
    PieceTypeRegistry(const PieceTypeRegistry&) = delete;
    PieceTypeRegistry& operator=(const PieceTypeRegistry&) = delete;

    std::unordered_map<std::string, std::unique_ptr<PieceTypeDefinition>> m_byId;
    std::unordered_map<char, const PieceTypeDefinition*>                 m_bySymbol;
};
