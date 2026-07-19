#pragma once

#include <string>
#include <fstream>
#include <sstream>

/// PieceStateConfig — נתוני פיזיקה לכל state של כלי.
/// נקרא מתוך config.json (מפתח "physics" בלבד).
/// שייך לשכבת הלוגיקה — אינו מכיר גרפיקה.
struct PieceStateConfig final {
    double speed_m_per_sec = 0.0;
    std::string next_state_when_finished = "idle";
};

// ═══════════════════════════════════════════════════
// עוזר פנימי — מחלץ ערך JSON פשוט
// ═══════════════════════════════════════════════════

namespace piece_config_detail {

/// מחלץ ערך של מפתח JSON פשוט.
inline std::string extract_json_value(const std::string& content,
                                       const std::string& key)
{
    auto pos = content.find('"' + key + '"');
    if (pos == std::string::npos) return "";
    pos = content.find(':', pos + key.size() + 2);
    if (pos == std::string::npos) return "";
    pos++; // skip ':'
    while (pos < content.size() &&
           (content[pos] == ' ' || content[pos] == '\t'))
        pos++;
    if (pos >= content.size()) return "";

    // String value (quoted)
    if (content[pos] == '"') {
        pos++; // skip opening quote
        std::string val;
        while (pos < content.size() && content[pos] != '"') {
            val.push_back(content[pos]);
            pos++;
        }
        return val;
    }

    // Numeric/boolean value (unquoted)
    std::string val;
    while (pos < content.size() && content[pos] != ',' &&
           content[pos] != '}' && content[pos] != ' ' &&
           content[pos] != '\t' && content[pos] != '\n' &&
           content[pos] != '\r')
    {
        val.push_back(content[pos]);
        pos++;
    }
    return val;
}

} // namespace piece_config_detail

/// טוען PieceStateConfig — מחלץ רק את מפתח "physics".
inline PieceStateConfig load_piece_state_config(
    const std::string& rootDir,
    const std::string& code,
    const std::string& stateName)
{
    PieceStateConfig cfg;

    std::string path = rootDir + "/" + code + "/states/" +
                       stateName + "/config.json";
    std::ifstream file(path);
    if (!file.is_open()) {
        return cfg; // defaults
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());

    std::string speedStr  = piece_config_detail::extract_json_value(content, "speed_m_per_sec");
    std::string nextState = piece_config_detail::extract_json_value(content, "next_state_when_finished");

    if (!speedStr.empty())  cfg.speed_m_per_sec = std::stod(speedStr);
    if (!nextState.empty()) cfg.next_state_when_finished = nextState;

    return cfg;
}
