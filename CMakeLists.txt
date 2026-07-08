cmake_minimum_required(VERSION 3.20)
project(KungFuChess VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# איתור כל קובצי ה-.cpp
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(KungFuChess ${SOURCES})

# אופציות לניפוי שגיאות — MSVC flags
if(MSVC)
    target_compile_options(KungFuChess PRIVATE /W4)
else()
    target_compile_options(KungFuChess PRIVATE -Wall -Wextra -Wpedantic)
endif()