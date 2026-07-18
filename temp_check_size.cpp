#include <opencv2/opencv.hpp>
#include <iostream>
int main() {
    cv::Mat img = cv::imread("src/graphics/board.png", cv::IMREAD_UNCHANGED);
    if (img.empty()) { std::cerr << "Cannot load" << std::endl; return 1; }
    std::cout << "board.png: " << img.cols << "x" << img.rows << " channels=" << img.channels() << std::endl;

    // check a sprite
    cv::Mat sprite = cv::imread("src/graphics/pieces2/RW/states/idle/sprites/1.png", cv::IMREAD_UNCHANGED);
    if (sprite.empty()) { std::cerr << "Cannot load sprite" << std::endl; return 1; }
    std::cout << "sprite: " << sprite.cols << "x" << sprite.rows << " channels=" << sprite.channels() << std::endl;

    return 0;
}
