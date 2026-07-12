# Kung-Fu Chess — C++20 Game Engine

## Overview
Turn-based chess variant where pieces move in real-time ("Kung-Fu style").
Current phase: **text-command prototype** (no GUI yet).

## Tech Stack
- **Language**: C++20
- **Build**: CMake 3.20+, MSVC 19.51+ (Visual Studio 2026)
- **No external dependencies** — standard library only
- **Test framework**: manual (VPL-compatible input/output tests)

## Build & Run
```bash
# Configure
cmake -S . -B build2 -G "Visual Studio 18 2026" -A x64

# Build
cmake --build build2 --config Debug

# Run with sample input
./build2/Debug/KungFuChess.exe < input.txt

# Full rebuild (when CMakeLists.txt changes)
rm -rf build2 && cmake -S . -B build2 -G "Visual Studio 18 2026" -A x64
```

## Project Structure
```
src/
├── main.cpp                          # Entry point: init registry, parse board, DSL dispatch
└── Model/
    ├── Position.h                    # Value Object: struct {row,col} + operator== (no board knowledge)
    ├── PieceColor.h                  # enum class PieceColor { White, Black }
    ├── PieceTypeDefinition.h         # struct: id, symbol, vector<MovementRule> — DATA only
    ├── MovementRule.h/.cpp           # struct: pattern + directions + maxSteps — DATA, not CODE
    ├── MoveGenerator.h/.cpp          # Static: generate(rule, pos, color, rows, cols, hasMoved) → vector<Position>
    ├── PieceTypeRegistry.h/.cpp      # Singleton: stores all piece type definitions (std + custom)
    ├── Piece.h/.cpp                  # Concrete piece: color + string typeId + pos — delegates to Registry+MoveGenerator
    ├── PieceFactory.h/.cpp           # Factory: create(token, row, col) → optional<Piece> (registry lookup, no switch)
    ├── Board.h/.cpp                  # Grid: vector<optional<Piece>>, place/remove/at/to_string(selected)
    ├── BoardParser.h/.cpp            # Static: parse(istream) → optional<Board> (reads "Board:" / "Commands:" format)
    ├── GameConfig.h                  # struct: boardRows, boardCols, cellSizePixels — all constants
    ├── PixelMapper.h/.cpp            # Pixel→cell coordinate conversion (separate from GameEngine)
    ├── RuleEngine.h/.cpp             # Read-only move validation: validate_move(from, to) → MoveValidation
    ├── GameEngine.h/.cpp             # Game coordination: selection, turns, clock, snapshots, game-end
    └── CommandInterpreter.h/.cpp     # Static: parse(string) → ParsedCommand {type, arg1, arg2} — parsing only, SRP
```

## Design Principles
- **DRY** — each piece of logic is implemented in only one place
- **SRP** — every class/function does only ONE thing
- **No hard-coded constants** in business logic — everything sits in configuration
- **Encapsulation** — classes/functions do not expose inner implementation details

## Coding Conventions
- **Naming**: PascalCase for classes, camelCase for methods/variables, UPPER_CASE for constants
- **Headers**: `.h` only (no `.hpp`), header guards with `#pragma once`
- **Comments**: Hebrew for explanations, English for code identifiers
- **Error handling**: `std::optional` over exceptions for expected failures
- **Const correctness**: mark everything `const` unless mutating
- **Position**: Value Object — knows nothing about board boundaries, pixels, or rules

## Architecture — Design Patterns

### Movement as DATA (Strategy Pattern)
Pieces don't contain movement code. Movement rules are pure data:
```
MovementRule { MovePattern(Step|Slide|Jump), vector<Direction>, maxSteps, canCapture }
PieceTypeDefinition { id, symbol, vector<MovementRule> }
```

| Piece  | Pattern | Directions      | Max Steps |
|--------|---------|-----------------|-----------|
| King   | Step    | 8 directions    | 1         |
| Queen  | Slide   | 8 directions    | unlimited |
| Rook   | Slide   | 4 cardinal      | unlimited |
| Bishop | Slide   | 4 diagonal      | unlimited |
| Knight | Jump    | 8 L-shapes      | 1         |
| Pawn   | Step    | forward         | 2 (initial double-step) |

### Factory Pattern
`PieceFactory::create(token, row, col)` — translates `"wK"` → `Piece{White, "king", {row,col}}`.
Uses `PieceTypeRegistry` for symbol→type lookup. No hard-coded `switch`.

### Singleton
`PieceTypeRegistry::instance()` — central registry of all piece types.
`register_standard()` loads the 6 standard chess pieces as data.

### Command Interpreter (SRP)
`CommandInterpreter::parse(line)` — text → `ParsedCommand`. Pure parsing, zero game logic.

### Pixel Mapper
`PixelMapper::to_cell(x, y, rows, cols)` — pixel → board cell. Geometry only, no game knowledge.
**Separate from GameEngine** — coordinate conversion is a caller responsibility.

### Rule Engine (Read-only Validation)
`RuleEngine::validate_move(from, to)` — answers: *is this move legal right now?*
- Read-only on Board — never mutates state
- Checks: outside_board, empty_source, friendly_destination, illegal_piece_move
- Delegates to `MoveGenerator` for piece-specific movement rules
- Returns `MoveValidation { is_valid: bool, reason: string }`
- Does NOT handle: check, pins, castling, en passant, promotion, or game-end

### Game Engine (Coordination)
`GameEngine` — app-service coordination:
- `select(pos)` — select a piece (validates turn)
- `request_move(from, to)` — validate via RuleEngine, execute move, detect game end
- `move_selected_to(target)` — selection logic + delegates to `request_move`
- `advance_clock(ms)` — game clock management
- `snapshot()` / `restore()` — save/restore full game state
- `is_game_over()` / `winner()` — king-capture victory condition

**What GameEngine does NOT do:**
- Pixel mapping (→ PixelMapper)
- Rendering (→ Board::to_string)
- DSL parsing (→ CommandInterpreter, dispatch in main.cpp)
- Piece-specific movement logic (→ MoveGenerator)
- Move validation (→ RuleEngine)

## Separation of Concerns Summary

| Concern              | Owner              |
|----------------------|---------------------|
| Pixel→cell mapping   | PixelMapper         |
| Text→command parsing | CommandInterpreter  |
| DSL dispatch         | main.cpp            |
| Board rendering      | Board::to_string()  |
| Move legality        | RuleEngine          |
| Raw move generation  | MoveGenerator       |
| Piece data           | PieceTypeRegistry   |
| Turn/selection/clock | GameEngine          |
| Game-end detection   | GameEngine          |
| Snapshots            | GameEngine          |

## Command Format (Text Input)
```
 Board:              ← leading spaces optional
wK wQ wR . .        ← "." = empty cell
wP wP wP wP
...
Commands:
click 50 50         ← pixel coords → cell (0,0)
click 150 50        ← pixel coords → cell (0,1)
wait 500            ← advance game clock by 500ms
print board         ← output current board state (selected piece in [brackets])
```

## Key Design Decisions
1. **string typeId** instead of enum — enables user-defined pieces at runtime
2. **Movement rules as data** — adding a new piece = one line of registration, zero code changes
3. **Each .cpp is exactly one class** — SRP, one responsibility per file
4. **Selection persists after move** — piece stays selected for chain moves
5. **Position is a pure Value Object** — no board boundary knowledge, no `is_valid()`
6. **Win condition: king capture** — simpler than checkmate; no check/pin/castling logic
7. **`GameEngine` name (not `Game`)** — consistent with the `*Engine` naming: RuleEngine, GameEngine

## Move Validation Flow
```
main.cpp (DSL dispatch)
  └─→ PixelMapper::to_cell(x, y)      — pixel → Position
  └─→ GameEngine::select(pos)          — select piece
  └─→ GameEngine::move_selected_to(t)  — selection logic
       └─→ GameEngine::request_move(from, to)
            ├─→ GameEngine: check game_over
            ├─→ RuleEngine::validate_move(from, to)   — read-only validation
            │    ├─→ outside_board / empty_source / friendly_destination
            │    └─→ Piece::get_raw_moves() → MoveGenerator::generate()
            ├─→ GameEngine: check current turn
            ├─→ GameEngine: execute move on Board
            └─→ GameEngine: detect king capture → game_over
```

## Future Extensibility Points
- User-defined pieces: load `PieceTypeRegistry` from JSON instead of `register_standard()`
- Custom movement: add `MovementRule` variants
- Real-time moves: `GameEngine::advance_clock()` will resolve in-flight move animations
- GUI: Observer pattern — Board notifies UI of state changes
- Advanced rules: check/pin detection, castling, en passant, promotion — extend RuleEngine

## VPL Submission
`vpl_submission.cpp` is a **standalone single-file** version for VPL uploads.
It contains inlined versions of: Position, Piece, Board, parseBoard, pixelToCell, printBoard, main.
No external dependencies, no Registry, no MovementRule — minimal implementation for tests only.

## Git
- Initialized on master, first commit: `b4e70a6`
- Author: YaffiAdler <yf546776@gmail.com>
- `.gitignore` excludes: build/, .vs/, *.obj, *.exe, CMakeFiles/, etc.
