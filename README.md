# Kung-Fu Chess — C++20 Game Engine

## Overview

Turn-based chess variant where pieces move in real-time ("Kung-Fu style"). Current phase: text-command prototype (no GUI yet).

## Tech Stack

- **Language:** C++20
- **Build:** CMake 3.20+, MSVC 19.51+ (Visual Studio 2026)
- **Dependencies:** Standard Library only
- **Testing:** Manual (VPL-compatible input/output tests)

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
├── main.cpp
└── Model/
    ├── Position
    ├── Piece
    ├── Board
    ├── PieceFactory
    ├── PieceTypeRegistry
    ├── MoveGenerator
    ├── RuleEngine
    ├── GameEngine
    ├── PixelMapper
    ├── BoardParser
    └── CommandInterpreter
```

## Architecture

- **Movement rules** are stored as data (`MovementRule`, `PieceTypeDefinition`)
- **Factory Pattern** for piece creation
- **Singleton** for piece type registry
- **Command Interpreter** for text command parsing
- **RuleEngine** performs read-only move validation
- **GameEngine** coordinates turns, selection, clock, snapshots and game state

## Design Principles

- DRY
- SRP
- Encapsulation
- Configuration instead of hard-coded constants
- Const correctness
- `std::optional` for expected failures

## Responsibilities

| Component | Responsibility |
|----------|----------------|
| PixelMapper | Pixel → board cell conversion |
| CommandInterpreter | Text command parsing |
| MoveGenerator | Raw move generation |
| RuleEngine | Move validation |
| PieceTypeRegistry | Piece definitions |
| GameEngine | Turns, selection, clock, snapshots, game state |
| Board | Board representation and rendering |

## Command Format

```
Board:
wK wQ wR . .
wP wP wP wP
...

Commands:
click 50 50
click 150 50
wait 500
print board
```

## Key Design Decisions

- `string typeId` instead of enum to support runtime-defined pieces
- Movement rules represented as data
- One class per `.cpp`
- Selection persists after move
- `Position` is a pure Value Object
- Win condition: king capture

## Future Extensibility

- Load piece definitions from JSON
- Additional movement rule types
- GUI integration
- Advanced chess rules
