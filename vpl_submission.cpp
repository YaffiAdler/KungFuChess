#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <optional>
#include <cctype>
#include <cstddef>

// ============================================================
//  Position
// ============================================================
struct Position {
    int row, col;
         [[nodiscard]] bool operator==(const Position& other) const noexcept {
        return row == other.row && col == other.col;
    }

    [[nodiscard]] bool is_valid(int num_rows, int num_cols) const noexcept {
        return row >= 0 && row < num_rows && col >= 0 && col < num_cols;
    }
    
};

// ============================================================
//  PieceColor / PieceType
// ============================================================
enum class PieceColor { White, Black };

// ============================================================
//  Piece (קל — just token string)
// ============================================================
struct Piece {
    PieceColor color;
    char symbol;   // 'K','Q','R','B','N','P'
    Position pos;
    bool hasMoved = false;

    std::string token() const {
        char c = (color == PieceColor::White) ? 'w' : 'b';
        return std::string{c, symbol};
    }
};

// ============================================================
//  Board
// ============================================================
class Board {
public:
    Board(int rows, int cols)
        : m_rows(rows), m_cols(cols), m_cells(rows * cols) {}

    int rows() const noexcept { return m_rows; }
    int cols() const noexcept { return m_cols; }

    const std::optional<Piece>& at(int r, int c) const {
        return m_cells[r * m_cols + c];
    }
    std::optional<Piece>& at(int r, int c) {
        return m_cells[r * m_cols + c];
    }
    std::optional<Piece>& at(Position p) { return at(p.row, p.col); }

    bool valid(Position p) const noexcept { return p.is_valid(m_rows, m_cols); }

    void place(Piece p) { at(p.pos) = std::move(p); }
    void remove(Position p) { at(p) = std::nullopt; }

private:
    int m_rows, m_cols;
    std::vector<std::optional<Piece>> m_cells;
};

// ============================================================
//  Token validation
// ============================================================
bool isValidToken(const std::string& token) {
    if (token == ".") return true;
    if (token.length() != 2) return false;
    char c = static_cast<char>(std::tolower(static_cast<unsigned char>(token[0])));
    if (c != 'w' && c != 'b') return false;
    char p = static_cast<char>(std::toupper(static_cast<unsigned char>(token[1])));
    return p == 'K' || p == 'Q' || p == 'R' || p == 'B' || p == 'N' || p == 'P';
}

// ============================================================
//  Create piece from token
// ============================================================
std::optional<Piece> createPiece(const std::string& token, int row, int col) {
    if (token == ".") return std::nullopt;
    PieceColor color = (std::tolower(token[0]) == 'w') ? PieceColor::White : PieceColor::Black;
    char sym = static_cast<char>(std::toupper(static_cast<unsigned char>(token[1])));
    return Piece{color, sym, {row, col}};
}

// ============================================================
//  Parse board from stdin
//  returns {board, success}
// ============================================================
std::pair<std::optional<Board>, bool> parseBoard() {
    std::string line;
    std::vector<std::vector<std::string>> rawRows;
    std::size_t expectedCols = 0;
    bool seenBoard = false;

    while (std::getline(std::cin, line)) {
        // Detect "Board:" header (may have leading whitespace)
        {
            std::string trimmed = line;
            // remove leading spaces
            while (!trimmed.empty() && trimmed[0] == ' ')
                trimmed.erase(0, 1);
            if (trimmed == "Board:") { seenBoard = true; continue; }
            if (trimmed == "Commands:") break;
        }

        if (!seenBoard) continue;  // skip until we see Board:

        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> row;
        while (ss >> token) {
            if (!isValidToken(token)) {
                std::cout << "ERROR UNKNOWN_TOKEN\n";
                return {{}, false};
            }
            row.push_back(token);
        }

        if (!row.empty()) {
            if (expectedCols == 0) expectedCols = row.size();
            else if (row.size() != expectedCols) {
                std::cout << "ERROR ROW_WIDTH_MISMATCH\n";
                return {{}, false};
            }
            rawRows.push_back(std::move(row));
        }
    }

    if (rawRows.empty()) return {{}, false};

    int numRows = static_cast<int>(rawRows.size());
    int numCols = static_cast<int>(expectedCols);
    Board board(numRows, numCols);

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {
            const auto& tok = rawRows[r][c];
            auto piece = createPiece(tok, r, c);
            if (piece) board.place(std::move(*piece));
        }
    }

    return {std::move(board), true};
}

// ============================================================
//  pixel_to_cell
// ============================================================
std::optional<Position> pixelToCell(int x, int y, const Board& board) {
    if (x < 0 || y < 0) return std::nullopt;
    int col = x / 100;
    int row = y / 100;
    Position p{row, col};
    if (board.valid(p)) return p;
    return std::nullopt;
}

// ============================================================
//  print board
// ============================================================
void printBoard(const Board& board) {
    for (int r = 0; r < board.rows(); ++r) {
        for (int c = 0; c < board.cols(); ++c) {
            if (c > 0) std::cout << ' ';
            const auto& cell = board.at(r, c);
            if (cell.has_value()) {
                std::cout << cell->token();
            } else {
                std::cout << "..";
            }
        }
        std::cout << '\n';
    }
}

// ============================================================
//  Main
// ============================================================
int main() {
    // Parse Board section
    auto [boardOpt, ok] = parseBoard();
    if (!ok || !boardOpt.has_value()) return 0;

    Board board = std::move(*boardOpt);
    std::optional<Position> selected;
    std::string line;

    // Process Commands
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd;
        if (!(iss >> cmd)) continue;

        if (cmd == "click") {
            int x = 0, y = 0;
            if (!(iss >> x >> y)) continue;

            auto cell = pixelToCell(x, y, board);
            if (!cell) continue;  // מחוץ ללוח — התעלם

            Position clicked = *cell;
            const auto& clickedCell = board.at(clicked);

            // ── אין בחירה ──
            if (!selected.has_value()) {
                if (clickedCell.has_value()) {
                    selected = clicked;
                }
                continue;
            }

            // ── יש בחירה ──
            Position from = *selected;
            const auto& fromCell = board.at(from);
            if (!fromCell.has_value()) { selected.reset(); continue; }

            if (clicked == from) continue;

            // קליק על כלי ידידותי ← החלף בחירה
            if (clickedCell.has_value() && clickedCell->color == fromCell->color) {
                selected = clicked;
                continue;
            }

            // מהלך
            Piece moving = *fromCell;
            board.remove(from);
            moving.pos = clicked;
            moving.hasMoved = true;
            board.place(std::move(moving));
            selected = clicked;
        }
        else if (cmd == "wait") {
            int ms = 0;
            iss >> ms;
            // accumulate game time (not used further in this simple version)
        }
        else if (cmd == "print") {
            std::string what;
            iss >> what;
            if (what == "board") {
                printBoard(board);
            }
        }
    }

    return 0;
}
