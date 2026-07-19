#include "img.hpp"
#include <iostream>
#include <stdexcept>

Img::Img() {
    // Constructor - img is automatically initialized as empty
}

Img& Img::read(const std::string& path,
               const std::pair<int, int>& size,
               bool keep_aspect,
               int interpolation) {
    img = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        throw std::runtime_error("Cannot load image: " + path);
    }

    if (size.first != 0 && size.second != 0) {  // Check if size is not empty
        int target_w = size.first;
        int target_h = size.second;
        int h = img.rows;
        int w = img.cols;

        if (keep_aspect) {
            double scale = std::min(static_cast<double>(target_w) / w,
                                   static_cast<double>(target_h) / h);
            int new_w = static_cast<int>(w * scale);
            int new_h = static_cast<int>(h * scale);
            cv::resize(img, img, cv::Size(new_w, new_h), 0, 0, interpolation);
        } else {
            cv::resize(img, img, cv::Size(target_w, target_h), 0, 0, interpolation);
        }
    }

    return *this;
}

void Img::draw_on(Img& other_img, int x, int y) {
    if (img.empty() || other_img.img.empty()) {
        throw std::runtime_error("Both images must be loaded before drawing.");
    }

    int h = img.rows;
    int w = img.cols;
    int H = other_img.img.rows;
    int W = other_img.img.cols;

    if (y + h > H || x + w > W) {
        throw std::runtime_error("Image does not fit at the specified position.");
    }

    cv::Mat roi = other_img.img(cv::Rect(x, y, w, h));

    if (img.channels() == 4) {
        // Alpha blending for BGRA source
        // Convert both to CV_32F for correct blending
        cv::Mat srcFloat, roiFloat;
        img(cv::Rect(0, 0, w, h)).convertTo(srcFloat, CV_32FC4);
        roi.convertTo(roiFloat, CV_32FC4);

        std::vector<cv::Mat> srcChannels(4);
        cv::split(srcFloat, srcChannels);

        std::vector<cv::Mat> roiChannels(4);
        cv::split(roiFloat, roiChannels);

        cv::Mat alpha = srcChannels[3] / 255.0; // normalize alpha to [0, 1]

        for (int c = 0; c < 3; ++c) {
            roiChannels[c] = srcChannels[c].mul(alpha) + roiChannels[c].mul(cv::Scalar(1.0) - alpha);
        }
        roiChannels[3] = cv::Mat::ones(roiChannels[3].size(), CV_32F);

        cv::Mat blendedFloat;
        cv::merge(roiChannels, blendedFloat);
        blendedFloat.convertTo(roi, CV_8UC4);
    } else {
        // Direct copy for BGR images (no alpha)
        img.copyTo(roi);
    }
}

void Img::put_text(const std::string& txt, int x, int y, double font_size,
                   const cv::Scalar& color, int thickness) {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }

    cv::putText(img, txt, cv::Point(x, y),
                cv::FONT_HERSHEY_SIMPLEX, font_size,
                color, thickness, cv::LINE_AA);
}

void Img::show() {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }

    cv::imshow("Image", img);
    cv::waitKey(0);
    cv::destroyAllWindows();
}
