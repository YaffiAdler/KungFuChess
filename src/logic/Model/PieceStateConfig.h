#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

/// ─────────────────────────────────────────────────
/// PieceStateConfig — נתוני פיזיקה לכל state של כלי.
/// נקרא מתוך config.json (מפתח "physics" בלבד).
/// שייך לשכבת הלוגיקה — אינו מכיר גרפיקה.
/// ─────────────────────────────────────────────────
struct PieceStateConfig final {
	double speed_m_per_sec = 0.0;
	std::string next_state_when_finished = "idle";
};

/// ─────────────────────────────────────────────────
/// SpriteAnimConfig — נתוני אנימציית sprite.
/// נקרא מתוך config.json (מפתח "graphics" בלבד).
/// שייך לשכבת הגרפיקה.
/// ─────────────────────────────────────────────────
struct SpriteAnimConfig final {
	int  frames_per_sec = 6;
	bool is_loop        = true;
};

// ═══════════════════════════════════════════════════
// קוראי JSON — כל אחד מחלץ רק את החלק שלו
// ═══════════════════════════════════════════════════

namespace detail {

/// מחלץ ערך של מפתח JSON פשוט — עוזר פנימי.
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

} // namespace detail

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

	std::string speedStr  = detail::extract_json_value(content, "speed_m_per_sec");
	std::string nextState = detail::extract_json_value(content, "next_state_when_finished");

	if (!speedStr.empty())  cfg.speed_m_per_sec = std::stod(speedStr);
	if (!nextState.empty()) cfg.next_state_when_finished = nextState;

	return cfg;
}

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

	std::string fpsStr  = detail::extract_json_value(content, "frames_per_sec");
	std::string loopStr = detail::extract_json_value(content, "is_loop");

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
