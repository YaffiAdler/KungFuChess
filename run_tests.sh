#!/bin/bash
# Kung-Fu Chess Test Runner — runs VPL submission against test files
# Usage: bash run_tests.sh

set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
# Use vpl_submission.cpp — single file with minimal logic, VPL-compatible
EXE="$PROJECT_DIR/vpl_build/Debug/vpl_test.exe"
VS_CMAKE='c:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'

# ── Build vpl_test ──
echo "==> Building vpl_test..."
cd "$PROJECT_DIR"
rm -rf vpl_build 2>/dev/null
cp CMakeLists.txt CMakeLists.txt.bak

cat > CMakeLists.txt << 'CMAKE_EOF'
cmake_minimum_required(VERSION 3.20)
project(VplTest VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(vpl_test vpl_submission.cpp)
if(MSVC)
    target_compile_options(vpl_test PRIVATE /W3)
else()
    target_compile_options(vpl_test PRIVATE -Wall -Wextra)
endif()
CMAKE_EOF

"$VS_CMAKE" -S . -B vpl_build -G "Visual Studio 18 2026" -A x64 > /dev/null 2>&1
"$VS_CMAKE" --build vpl_build --config Debug 2>&1 | tail -1
cp CMakeLists.txt.bak CMakeLists.txt
rm -f CMakeLists.txt.bak
echo ""

# ── Run tests ──
PASS=0
FAIL=0

for input_file in tests/test_*.txt; do
    test_name=$(basename "$input_file" .txt)
    expected_file="tests/expected/${test_name}.expected.txt"

    if [ ! -f "$expected_file" ]; then
        echo "  ⚠️  $test_name — no expected file, skipping"
        continue
    fi

    actual=$(cat "$input_file" | "$EXE" 2>&1 | sed 's/\r$//')
    expected=$(cat "$expected_file" | sed 's/\r$//')

    if [ "$actual" = "$expected" ]; then
        echo "  ✅ $test_name"
        PASS=$((PASS + 1))
    else
        echo "  ❌ $test_name"
        echo "     Expected:"
        echo "$expected" | sed 's/^/       | /'
        echo "     Got:"
        echo "$actual" | sed 's/^/       | /'
        FAIL=$((FAIL + 1))
    fi
done

echo ""
echo "Results: $PASS passed, $FAIL failed ($((PASS + FAIL)) total)"
