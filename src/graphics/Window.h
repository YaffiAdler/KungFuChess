#pragma once

#include "../logic/Model/GameConfig.h"
#include "../logic/Controller/Controller.h"
#include "Renderer.h"
#include "InputHandler.h"
#include "img.hpp"
#include <string>

class GameEngine;

/// חלון המשחק — מנהל את לולאת האירועים בלבד.
///
/// SRP: תפקיד יחיד — לנהל את לולאת ה-cv::waitKey,
/// לקשר בין InputHandler ↔ Controller ↔ Renderer ↔ GameEngine,
/// ולתאם ציור מחדש בכל איטרציה.
///
/// אינו מכיל לוגיקת ציור (Renderer) או לוגיקת קלט (InputHandler → Controller).
class Window final {
public:
    explicit Window(GameConfig config);
    ~Window();

    /// חיבור ל-GameEngine (נעשה אחרי הבנייה)
    void set_engine(GameEngine* engine);

    /// לולאת המשחק הראשית — מציגה חלון, מטפלת בקלט, מציירת
    void run();

    // callback סטטי עבור OpenCV
    static void mouse_callback(int event, int x, int y, int flags, void* userdata);

private:
    GameConfig   m_config;
    Img          m_screen;        // buffer המסך
    Renderer     m_renderer;
    Controller   m_controller;
    InputHandler m_input;
    GameEngine*  m_engine = nullptr;
    std::string  m_windowName;
};
