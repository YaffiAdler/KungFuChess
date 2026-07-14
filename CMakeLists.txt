cmake_minimum_required(VERSION 3.20)
project(KungFuChess VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# איתור כל קובצי ה-.cpp
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(KungFuChess ${SOURCES})

# ── OpenCV 4.5.1 ──
set(OPENCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cpp/OpenCV_451")
set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/include")
set(OPENCV_LIB_DIR "${OPENCV_DIR}/bin")

target_include_directories(KungFuChess PRIVATE ${OPENCV_INCLUDE_DIR})
target_link_directories(KungFuChess PRIVATE ${OPENCV_LIB_DIR})

target_link_libraries(KungFuChess
    $<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.lib>
    $<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    $<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    $<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.lib>
)

# העתקת OpenCV DLL ל-output directory
if(WIN32)
    add_custom_command(TARGET KungFuChess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.dll>
        $<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<TARGET_FILE_DIR:KungFuChess>)
endif()

# ── אופציות קומפילציה ──
if(MSVC)
    target_compile_options(KungFuChess PRIVATE /W4)
else()
    target_compile_options(KungFuChess PRIVATE -Wall -Wextra -Wpedantic)
endif()

message(STATUS "Found source files: ${SOURCES}")
