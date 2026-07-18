# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
# Configure (use build2/ — build/ and build4/ also exist but are older)
cmake -S . -B build2 -G "Visual Studio 18 2026" -A x64

# Build
cmake --build build2 --config Debug

# Run (GUI mode — game window)
./build2/Debug/KungFuChess.exe

# Full rebuild after CMakeLists.txt changes
rm -rf build2 && cmake -S . -B build2 -G "Visual Studio 18 2026" -A x64 && cmake --build build2 --config Debug
```

## Tests

Tests are **text-based input/output comparison** (no test framework). Each test is a pair: `tests/test_*.txt` (input) and `tests/expected/test_*.expected.txt` (expected output).

```bash
# Run all tests
bash run_tests.sh

# The script temporarily swaps CMakeLists.txt to build vpl_submission.cpp as a standalone exe
# Requires: Visual Studio 2026 CMake at the hardcoded path inside run_tests.sh
```

## Project Architecture

**Kung-Fu Chess** —  chess variant where pieces move in real-time with visible animation. C++20, CMake, OpenCV 4.5.1 for rendering.

### Dependency: OpenCV 4.5.1

Bundled at `cpp/OpenCV_451/` (headers + prebuilt libs/DLLs). The `Img` class (`src/graphics/img.hpp`) wraps `cv::Mat` for loading, alpha-blended drawing, and text rendering. OpenCV is used **only in the graphics layer** — Logic never touches it.

### Architecture Layers (strict dependency: Graphics → Logic)

```
src/
├── main.cpp                          # Entry: init registry, parse board, create Window
├── logic/
│   ├── Model/                        # Pure data + game rules (NO graphics, NO input)
│   │   ├── Position.h                # Value Object: {row, col} — no board knowledge
│   │   ├── PieceColor.h              # enum: White, Black
│   │   ├── Piece.h                   # Concrete piece: color + string typeId + pos + state
│   │   ├── PieceTypeDefinition.h     # Struct: id, symbol, vector<MovementRule> (DATA only)
│   │   ├── MovementRule.h/.cpp       # Struct: pattern(Step/Slide/Jump) + directions + maxSteps
│   │   ├── MoveGenerator.h/.cpp      # Static: generate moves from rules (NO board/validation)
│   │   ├── PieceRules.h/.cpp         # Static: path-blocking checks only (delegates to MoveGenerator)
│   │   ├── PieceTypeRegistry.h/.cpp  # Singleton: stores all piece type definitions
│   │   ├── PieceFactory.h/.cpp       # Factory: create(token,row,col) → Piece via Registry lookup
│   │   ├── Board.h/.cpp              # Grid: vector<optional<Piece>>, place/remove/at/to_string
│   │   ├── BoardParser.h/.cpp        # Static: parse(istream) → optional<Board>
│   │   ├── RuleEngine.h/.cpp         # Read-only move validation: validate_move(from,to) → MoveValidation
│   │   ├── RealTimeArbiter.h/.cpp    # Manages active motion: start→tick→complete (does NOT update Board)
│   │   ├── GameEngine.h/.cpp         # Coordination: selection, turns, snapshots, game-end
│   │   ├── GameConfig.h              # All constants: boardRows/Cols, cellSizePixels, msPerCell
│   │   └── CommandInterpreter.h/.cpp # Static: parse(text) → ParsedCommand (text-command mode only)
│   └── Controller/
│       ├── Controller.h/.cpp         # Translates clicks→game actions, mediates Arbiter timing
│       └── PixelMapper.h/.cpp        # Pixel→cell coordinate conversion
└── graphics/
    ├── Window.h/.cpp                 # Event loop (cv::waitKey), wires Input→Controller→Renderer→Engine
    ├── img.hpp/.cpp                  # OpenCV wrapper: load, alpha-blend, text (the ONLY OpenCV dependency)
    ├── Renderer.h/.cpp               # Full-frame drawing: board + pieces + selection highlight
    ├── PieceRenderer.h/.cpp          # Per-piece sprite loading/caching/drawing (supports interpolation)
    └── InputHandler.h/.cpp           # Raw keyboard/mouse → Controller (no game logic)
```

### Key Design Decisions

1. **Movement as DATA** — pieces have no movement code. `PieceTypeDefinition` holds a vector of `MovementRule` structs (pattern + directions + maxSteps). Adding a piece = one line of registration, zero code changes.
2. **`string typeId`** (not enum) — enables user-defined pieces at runtime.
3. **King-capture victory** — simpler than checkmate. No check/pin/castling/en-passant logic.
4. **Position is a pure Value Object** — no `is_valid()`, no board boundaries, no pixel knowledge. Just `{row, col}` with `operator==`.
5. **RealTimeArbiter owns Motion, not Board** — during animation the moving piece lives in `Motion`, removed from Board. On completion, Arbiter signals commit; Controller calls `GameEngine::commit_move`.
6. **Renderer renders from GameEngine snapshot + Arbiter** — it reads read-only state and interpolates piece positions. Never mutates game state.
7. **`GameConfig` struct** holds all tunable constants — no magic numbers in business logic.

### Move Flow (GUI mode)

```
InputHandler::register_click(x,y)     // from mouse callback
  → InputHandler::process_click()
    → Controller::handle_click(x,y)
      → PixelMapper::to_cell()        // pixel → Position
      → GameEngine::move_selected_to(target)
        → RuleEngine::validate_move() // read-only check
        → sets busy, returns motionStarted=true
      → RealTimeArbiter::start_motion(from,to,piece,msPerCell)
Window::run() loop:
  → Controller::tick(engine)
    → RealTimeArbiter::tick(deltaMs)  // advances elapsed, returns completed flag
    → GameEngine::commit_move()       // on completion, updates Board
  → Renderer::render_frame(screen, engine, arbiter)  // interpolates if motion active
```

### Text Command Interface (legacy/VPL)

```
Board:              ← leading spaces optional
wK wQ wR . .        ← "." = empty cell
...
Commands:
click 50 50         ← pixel coords
print board         ← output board with [selected] piece
```

Parsed by `CommandInterpreter::parse()` in text mode. `vpl_submission.cpp` is a standalone single-file version for VPL uploads (does not exist in current tree — was removed; tests build against it via `run_tests.sh` that temporarily swaps CMakeLists.txt).

### Coding Conventions

- **Naming**: `PascalCase` classes, `camelCase` methods/variables, `UPPER_CASE` constants
- **Headers**: `.h` only, `#pragma once`
- **Error handling**: `std::optional` for expected failures, exceptions for truly exceptional cases
- **Const correctness**: mark everything `const` unless mutating
- **Comments**: Hebrew for explanations, English for code identifiers
- **One class per `.cpp`** — each file has a single responsibility

### Forbidden Patterns (from docs/rulesarchitecture.md)

- Logic layer must NEVER depend on graphics classes
- Renderer must NEVER modify game state
- Graphics must NEVER implement game rules
- Tests must NOT bypass public APIs
- No global mutable state, no god classes, no hidden dependencies
