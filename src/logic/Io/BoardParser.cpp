#include "BoardParser.h"
#include "../Model/PieceFactory.h"
#include <sstream>
#include <stdexcept>

std::string BoardParser::m_lastError;

// ─────────────────────────────────────────────
//  parse
// ─────────────────────────────────────────────
std::optional<Board> BoardParser::parse(std::istream& in, std::ostream& errStream) {
    m_lastError.clear();
    std::string line;
    std::vector<std::vector<std::string>> rawRows;
    std::size_t expectedCols = 0;

    // ── שלב 1: קריאת שורות הלוח ──
    while (std::getline(in, line)) {
        // Trim leading spaces for header detection
        std::string trimmed = line;
        while (!trimmed.empty() && trimmed[0] == ' ')
            trimmed.erase(0, 1);

        if (trimmed == "Commands:") break;
        if (trimmed == "Board:" || line.empty()) continue;

        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> row;

        while (ss >> token) {
            if (!PieceFactory::is_valid_token(token)) {
                m_lastError = "ERROR UNKNOWN_TOKEN";
                errStream << m_lastError << '\n';
                return std::nullopt;
            }
            row.push_back(token);
        }

        if (!row.empty()) {
            if (expectedCols == 0) expectedCols = row.size();
            else if (row.size() != expectedCols) {
                m_lastError = "ERROR ROW_WIDTH_MISMATCH";
                errStream << m_lastError << '\n';
                return std::nullopt;
            }
            rawRows.push_back(std::move(row));
        }
    }

    if (rawRows.empty()) {
        m_lastError = "ERROR EMPTY_BOARD";
        errStream << m_lastError << '\n';
        return std::nullopt;
    }

    // ── שלב 2: בניית Board ──
    int numRows = static_cast<int>(rawRows.size());
    int numCols = static_cast<int>(expectedCols);
    Board board(numRows, numCols);

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {
            const auto& token = rawRows[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)];
            if (token == ".") continue;  // תא ריק

            auto piece = PieceFactory::create(token, r, c);
            if (piece.has_value()) {
                board.place(std::move(*piece));
            }
        }
    }

    return board;
}

// ─────────────────────────────────────────────
//  parse_or_throw
// ─────────────────────────────────────────────
Board BoardParser::parse_or_throw(std::istream& in) {
    std::ostringstream dummy;
    auto result = parse(in, dummy);
    if (!result.has_value()) {
        throw std::runtime_error(m_lastError.empty() ? "Board parse failed" : m_lastError);
    }
    return std::move(*result);
}
