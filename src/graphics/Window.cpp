#include "Window.h"
#include <opencv2/highgui.hpp>
#include <iostream>

// ─────────────────────────────────────────────
//  mouse_callback — static, מעביר ל-InputHandler
// ─────────────────────────────────────────────
void Window::mouse_callback(int event, int x, int y, int /*flags*/, void* userdata) {
    if (event != cv::EVENT_LBUTTONDOWN) return;

    auto* handler = static_cast<InputHandler*>(userdata);
    if (handler) {
        handler->register_click(x, y);
    }
}

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
Window::Window(GameConfig config)
    : m_config(config)
    , m_renderer("src/graphics/board.png",
                 "src/graphics/pieces2",
                 config.cellSizePixels, config.cellSizePixels)
    , m_pixelMapper(config)
    , m_input(m_pixelMapper)
    , m_windowName("Kung-Fu Chess")
{}

Window::~Window() {
    cv::destroyAllWindows();
}

// ─────────────────────────────────────────────
//  set_engine
// ─────────────────────────────────────────────
void Window::set_engine(GameEngine* engine) {
    m_engine = engine;
}

// ─────────────────────────────────────────────
//  run — לולאת אירועים בלבד
// ─────────────────────────────────────────────
void Window::run() {
    if (!m_engine) {
        std::cerr << "Window::run() — no GameEngine set!" << std::endl;
        return;
    }

    // אתחול Renderer — טוען לוח ומתאים גדלי תאים
    m_renderer.init(*m_engine);

    // המסך מתחיל כעותק של תמונת הלוח
    m_screen.read("src/graphics/board.png");

    // חלון + callback עם userdata = &m_input
    cv::namedWindow(m_windowName, cv::WINDOW_NORMAL);
    cv::setMouseCallback(m_windowName, mouse_callback, &m_input);

    // ציור התחלתי
    m_renderer.render_frame(m_screen, *m_engine);
    cv::imshow(m_windowName, m_screen.get_mat());

    // ═══ לולאה ראשית ═══
    while (true) {
        int key = cv::waitKey(30);

        // מקלדת
        if (!m_input.process_key(key, *m_engine)) {
            break;  // ESC / q
        }

        // עכבר
        m_input.process_click(*m_engine);

        // ציור מחדש
        m_screen.read("src/graphics/board.png");
        m_renderer.render_frame(m_screen, *m_engine);
        cv::imshow(m_windowName, m_screen.get_mat());
    }
}
