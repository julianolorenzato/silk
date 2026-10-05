#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "config.h"
#include "model.h"
#include "tgaimage.h"

struct DeviceCoord {
    int x, y;

    DeviceCoord() = default;
    DeviceCoord(int x, int y) : x(x), y(y) {}
};

void draw_frame_padding(TGAImage& framebuffer, int padding = 12) {
    int size = framebuffer.width();

    for (int i = 0; i < size; i++) {
        int limit = std::min(std::min(i, size - i), padding);

        for (int j = 0; j < limit; j++) {
            framebuffer.set(i, j, GREEN);
            framebuffer.set(j, i, YELLOW);
            framebuffer.set(size - i, size - j, BLUE);
            framebuffer.set(size - j, size - i, RED);
        }
    }
}

// Bresenham's algorithm
void line(TGAImage& fb, DeviceCoord a, DeviceCoord b, TGAColor color) {
    // Transpose points coordinates if its very steep.
    auto dx = std::abs(b.x - a.x), dy = std::abs(b.y - a.y);
    auto steep = dy > dx;
    if (steep) {
        std::swap(a.x, a.y);
        std::swap(b.x, b.y);
    }

    // Canonicalize the direction of drawing,
    // it should starts at the lower x coordinate pointer.
    if (a.x > b.x) {
        std::swap(a, b);
    }

    for (int x = a.x; x <= b.x; x++) {
        double t = (x - a.x) / static_cast<double>(b.x - a.x);

        int y = std::round(a.y + (b.y - a.y) * t);

        if (steep)
            fb.set(y, x, color);
        else
            fb.set(x, y, color);
    }
}

// A kind of scanline rasterization algorithm. But scanning columns xD.
void triangle2(TGAImage& fb, DeviceCoord a, DeviceCoord b, DeviceCoord c, TGAColor color) {
    if (a.x > b.x) std::swap(a, b);
    if (a.x > c.x) std::swap(a, c);
    if (b.x > c.x) std::swap(b, c);

    auto tAb = [a, b](int x) { return (x - a.x) / static_cast<double>(b.x - a.x); };
    auto tAc = [a, c](int x) { return (x - a.x) / static_cast<double>(c.x - a.x); };
    auto tBc = [b, c](int x) { return (x - b.x) / static_cast<double>(c.x - b.x); };
    auto yAb = [a, b](double t) { return std::round(a.y + (b.y - a.y) * t); };
    auto yAc = [a, c](double t) { return std::round(a.y + (c.y - a.y) * t); };
    auto yBc = [b, c](double t) { return std::round(b.y + (c.y - b.y) * t); };

    for (int x = a.x; x <= c.x; x++) {
        auto mAb = yAb(tAb(b.x));
        auto mAc = yAc(tAc(b.x));
        auto mBc = yBc(tBc(b.x));

        int top, bot;
        if (b.x >= x) {
            if (mAb > mAc) {
                top = yAb(tAb(x));
                bot = yAc(tAc(x));
            } else {
                top = yAc(tAc(x));
                bot = yAb(tAb(x));
            }
        } else {
            if (mBc > mAc) {
                top = yBc(tBc(x));
                bot = yAc(tAc(x));
            } else {
                top = yAc(tAc(x));
                bot = yBc(tBc(x));
            }
        }

        for (int y = bot; y <= top; y++) {
            fb.set(x, y, color);
        }

        // if (b.x >= x) {
        //     for (int y = a.y; y <= top; y++) {
        //         fb.set(x, y, color);
        //     }
        // } else {
        // }
    }
}

std::function<int(double t)> createX(DeviceCoord a, DeviceCoord b) {
    return [a, b](double t) { return std::round(a.y + (b.y - a.y) * t); };
}

std::function<double(int x)> createTFromX(DeviceCoord a, DeviceCoord b) {
    return [a, b](int x) { return (x - a.x) / static_cast<double>(b.x - a.x); };
}

// y = a.y + t * (b.y - a.y)
// x = a.x + t * (b.x - a.x)
//
// t = (x - ax) / (b.x - a.x)
std::function<int(int y)> createXFromY(DeviceCoord a, DeviceCoord b) {
    // Segmento horizontal: x não é função de y.
    // Retorna a.x como fallback; ajuste conforme sua necessidade.
    if (a.y == b.y) {
        return [a](int) { return a.x; };
    }

    // Pré-calcula a razão dx/dy uma única vez
    const double slopeInv = static_cast<double>(b.x - a.x) / (b.y - a.y);

    return
        [a, slopeInv](int y) { return static_cast<int>(std::lround(a.x + (y - a.y) * slopeInv)); };
}

// Scanline algorithm.
void triangle(TGAImage& fb, DeviceCoord a, DeviceCoord b, DeviceCoord c, TGAColor color) {
    // Sort coordinates by the y value.
    if (a.y > b.y) std::swap(a, b);
    if (a.y > c.y) std::swap(a, c);
    if (b.y > c.y) std::swap(b, c);

    auto xAB = createXFromY(a, b), xBC = createXFromY(b, c), xAC = createXFromY(a, c);

    // if (a.y != b.y)
    // Iterate through lines from bottom to middle (first half).
    for (int y = a.y; y < b.y; y++) {
        auto xStart = std::min(xAB(y), xAC(y));
        auto xEnd = std::max(xAB(y), xAC(y));

        // Iterate through current line.
        for (int x = xStart; x <= xEnd; x++) {
            fb.set(x, y, color);
        };
    }

    // if (b.y != c.y)
    // Iterate through lines from middle to top (second half).
    for (int y = b.y; y <= c.y; y++) {
        auto xStart = std::min(xBC(y), xAC(y));
        auto xEnd = std::max(xBC(y), xAC(y));

        // Iterate through current line.
        for (int x = xStart; x <= xEnd; x++) {
            fb.set(x, y, color);
        };
    }
}

// Orthogonal projection and transformed to device coordinates
DeviceCoord project(Vertex v, int width, int height) {
    double x = (v.x + 1.0) * width * 0.5;
    double y = (v.y + 1.0) * height * 0.5;

    return DeviceCoord(x, y);
}

int main(const int argc, const char** argv) {
    Config config(argc, argv);

    constexpr int SIZE = 1024;
    constexpr int WIDTH = SIZE;
    constexpr int HEIGHT = SIZE;
    TGAImage framebuffer(WIDTH, HEIGHT, TGAImage::RGB);

    // draw_frame_padding(framebuffer, 2);

    auto model = Model(config.getModelpath());

    std::cout << model << '\n';

    for (auto [a, b, c] : model.faces()) {
        auto va = model.vertices()[a];
        auto vb = model.vertices()[b];
        auto vc = model.vertices()[c];

        auto dca = project(va, WIDTH, HEIGHT);
        auto dcb = project(vb, WIDTH, HEIGHT);
        auto dcc = project(vc, WIDTH, HEIGHT);

        triangle(framebuffer, dca, dcb, dcc, TGAColor::random());
    }

    // auto p1 = DeviceCoord(7, 45);
    // auto p2 = DeviceCoord(35, 100);
    // auto p3 = DeviceCoord(45, 60);
    // auto p4 = DeviceCoord(120, 35);
    // auto p5 = DeviceCoord(90, 5);
    // auto p6 = DeviceCoord(45, 110);
    // auto p7 = DeviceCoord(115, 83);
    // auto p8 = DeviceCoord(80, 90);
    // auto p9 = DeviceCoord(85, 120);

    // triangle(framebuffer, p1, p2, p3, RED);
    // triangle(framebuffer, p4, p5, p6, WHITE);
    // triangle(framebuffer, p7, p8, p9, GREEN);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
