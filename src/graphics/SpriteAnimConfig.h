#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

/// SpriteAnimConfig — נתוני אנימציית sprite.
/// נקרא מתוך config.json (מפתח "graphics" בלבד).
/// שייך לשכבת הגרפיקה.
struct SpriteAnimConfig final {
    int  frames_per_sec = 6;
    bool is_loop        = true;
};

// ═══════════════════════════════════════════════════
// עוזר פנימי — מחלץ ערך JSON פשוט
// ═══════════════════════════════════════════════════

namespace {

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

} // anonymous namespace

/// טוען SpriteAnimConfig — מחלץ רק את מפתח "graphics".
inline SpriteAnimConfig load_sprite_anim_config(
    const std::string& rootDir,
    const std::string& code,
    const std::string& stateName)
{
    SpriteAnimConfig cfg;

    std::string path = rootDir + "/" + code + "/states/" +
                       stateName + "/config.json";
    std::ifstream file(path);
    if (!file.is_open()) {
        return cfg; // defaults
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());

    std::string fpsStr  = extract_json_value(content, "frames_per_sec");
    std::string loopStr = extract_json_value(content, "is_loop");

    if (!fpsStr.empty()) {
        cfg.frames_per_sec = std::stoi(fpsStr);
    }
    if (!loopStr.empty()) {
        std::string lower = loopStr;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        cfg.is_loop = (lower == "true");
    }

    return cfg;
}
