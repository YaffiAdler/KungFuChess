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
# Run
./build2/Debug/KungFuChess.exe < input.txt
```

## Project Structure
```
src/
├── main.cpp                          # Entry point: init registry, parse board, command loop
└── Model/
    ├── Position.h                    # struct {row,col} + is_valid() + operator==
    ├── PieceColor.h                  # enum class PieceColor { White, Black }
    ├── PieceTypeDefinition.h         # struct: id, symbol, vector<MovementRule> — DATA only
    ├── MovementRule.h/.cpp           # struct: pattern + directions + maxSteps — DATA, not CODE
    ├── MoveGenerator.h/.cpp          # Static: generate(rule, pos, color, rows, cols, hasMoved) → vector<Position>
    ├── PieceTypeRegistry.h/.cpp      # Singleton: stores all piece type definitions (std + custom)
    ├── Piece.h/.cpp                  # Concrete piece: color + string typeId + pos — delegates to Registry+MoveGenerator
    ├── PieceFactory.h/.cpp           # Factory: create(token, row, col) → optional<Piece> (registry lookup, no switch)
    ├── Board.h/.cpp                  # Grid: vector<optional<Piece>>, place/remove/at/to_string
    ├── BoardParser.h/.cpp            # Static: parse(istream) → optional<Board> (reads "Board:" / "Commands:" format)
    ├── GameConfig.h                  # struct: boardRows, boardCols, cellSizePixels — all constants
    ├── Game.h/.cpp                   # Game state: board + selection + clock + command dispatch
    └── CommandInterpreter.h/.cpp     # Static: parse(string) → ParsedCommand {type, arg1, arg2} — parsing only, SRP
```

## Architecture — Design Patterns

### Movement as DATA (Strategy Pattern)
Pieces don't contain movement code. Movement rules are pure data:
```
MovementRule { MovePattern(Step|Slide|Jump), vector<Direction>, maxSteps, canCapture }
PieceTypeDefinition { id, symbol, vector<MovementRule> }
```
- **King**: step 8 dirs
- **Queen**: slide 8 dirs
- **Rook**: slide 4 cardinal dirs
- **Bishop**: slide 4 diagonal dirs
- **Knight**: jump 8 L-shapes
- **Pawn**: step forward (maxSteps=2 for initial double-step)

### Factory Pattern
`PieceFactory::create(token, row, col)` — translates "wK" → Piece{White, "king", {row,col}}
Uses `PieceTypeRegistry` for symbol→type lookup. No hard-coded switch.

### Singleton
`PieceTypeRegistry::instance()` — central registry of all piece types.
`register_standard()` loads the 6 standard chess pieces as data.

### Command Interpreter (SRP)
`CommandInterpreter::parse(line)` — text→ParsedCommand. Pure parsing, zero game logic.

### Game Facade
`Game::execute_command(line)` — single entry point for all commands (click/wait/print).

## Command Format (Text Input)
```
 Board:              ← leading spaces optional
wK wQ wR . .        ← ".." = empty cell
wP wP wP wP
...
Commands:
click 50 50         ← pixel coords → cell (0,0)
click 150 50        ← pixel coords → cell (0,1)
wait 500            ← advance game clock by 500ms
print board         ← output current board state
```

## Key Design Decisions
1. **string typeId** instead of enum — enables user-defined pieces at runtime
2. **Movement rules as data** — adding a new piece = one line of registration, zero code changes
3. **Each .cpp is exactly one class** — SRP, one responsibility per file
4. **Selection persists after move** — piece stays selected for chain moves
5. **`..` for empty cells** — VPL-compatible board output format

## Future Extensibility Points
- User-defined pieces: load `PieceTypeRegistry` from JSON instead of `register_standard()`
- Custom movement: add `MovementRule` variants (e.g., "reverse at edge" for Shlomi-style pawns)
- Real-time moves: `Game::handle_wait()` will resolve in-flight move animations
- GUI: Observer pattern — Board notifies UI of state changes

## VPL Submission
`vpl_submission.cpp` is a **standalone single-file** version for VPL uploads.
It contains inlined versions of: Position, Piece, Board, parseBoard, pixelToCell, printBoard, main.
No external dependencies, no Registry, no MovementRule — minimal implementation for tests only.

## Git
- Initialized on master, first commit: `b4e70a6`
- Author: YaffiAdler <yf546776@gmail.com>
- `.gitignore` excludes: build/, .vs/, *.obj, *.exe, CMakeFiles/, etc.
