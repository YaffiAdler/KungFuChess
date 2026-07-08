#include "CommandInterpreter.h"
#include <sstream>

ParsedCommand CommandInterpreter::parse(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd;
    if (!(iss >> cmd)) {
        return {CommandType::Unknown, 0, 0, line};
    }

    if (cmd == "click") {
        int x = 0, y = 0;
        if (!(iss >> x >> y)) {
            return {CommandType::Unknown, x, y, line};  // Invalid arguments
        }
        // וידוא שאין ארגומנטים מיותרים
        std::string extra;
        if (iss >> extra) {
            return {CommandType::Unknown, x, y, line};  // Invalid arguments
        }
        return {CommandType::Click, x, y, line};
    }

    if (cmd == "wait") {
        int ms = 0;
        if (!(iss >> ms)) {
            return {CommandType::Unknown, ms, 0, line};  // Invalid arguments
        }
        if (ms < 0) {
            return {CommandType::Unknown, ms, 0, line};  // Invalid arguments
        }
        std::string extra;
        if (iss >> extra) {
            return {CommandType::Unknown, ms, 0, line};  // Invalid arguments
        }
        return {CommandType::Wait, ms, 0, line};
    }

    if (cmd == "print") {
        std::string what;
        if (!(iss >> what) || what != "board") {
            return {CommandType::Unknown, 0, 0, line};  // Unknown command
        }
        std::string extra;
        if (iss >> extra) {
            return {CommandType::Unknown, 0, 0, line};  // Invalid arguments
        }
        return {CommandType::PrintBoard, 0, 0, line};
    }

    return {CommandType::Unknown, 0, 0, line};
}
