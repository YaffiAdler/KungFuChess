#include "Window.h"
#include <opencv2/highgui.hpp>
#include <iostream>
#include <chrono>

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
    , m_controller(config)
    , m_input(m_controller)
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
//  run — לולאת אירועים + tick
// ─────────────────────────────────────────────
void Window::run() {
    if (!m_engine) {
        std::cerr << "Window::run() — no GameEngine set!" << std::endl;
        return;
    }

    // אתחול Renderer — טוען לוח ומתאים גדלי תאים
    m_renderer.init(*m_engine);

    // חלון + callback עם userdata = &m_input
    cv::namedWindow(m_windowName, cv::WINDOW_NORMAL);
    cv::setMouseCallback(m_windowName, mouse_callback, &m_input);

    // ציור התחלתי
    m_renderer.render_frame(m_screen, *m_engine, m_controller.arbiter());
    cv::imshow(m_windowName, m_screen.get_mat());

    // ═══ לולאה ראשית ═══
    using clock = std::chrono::steady_clock;
    auto lastFrameTime = clock::now();

    while (true) {
        int key = cv::waitKey(1);  // 1ms poll — tick handles timing

        // חישוב delta
        auto now   = clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrameTime);
        lastFrameTime = now;
        int deltaMs = static_cast<int>(delta.count());
        if (deltaMs < 1) deltaMs = 1;  // floor safeguard

        // ── 1. Tick — קידום זמן תנועה + state machines + אנימציה ──
        m_controller.tick(deltaMs, *m_engine);
        m_renderer.tick_animations(deltaMs);

        // ── 2. קלט — מקלדת ──
        if (!m_input.process_key(key, *m_engine)) {
            break;  // ESC / q
        }

        // ── 3. קלט — עכבר ──
        m_input.process_click(*m_engine);

        // ── 4. ציור ──
        m_renderer.render_frame(m_screen, *m_engine, m_controller.arbiter());
        cv::imshow(m_windowName, m_screen.get_mat());
    }
}
