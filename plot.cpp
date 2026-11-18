#include "plot.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <limits>

namespace {

struct Color { Uint8 r, g, b; };

const Color kPalette[] = {
    {31, 119, 180}, {214, 39, 40}, {44, 160, 44},
    {255, 127, 14}, {148, 103, 189}, {140, 86, 75},
};

// "Красивый" шаг сетки: 1, 2 или 5 * 10^k, примерно target делений на диапазон.
double niceStep(double range, int target)
{
    if (range <= 0) return 1.0;
    double raw = range / target;
    double mag = std::pow(10.0, std::floor(std::log10(raw)));
    double f = raw / mag;
    double nice = (f < 1.5) ? 1 : (f < 3.5) ? 2 : (f < 7.5) ? 5 : 10;
    return nice * mag;
}

void formatTick(char* buf, size_t n, double v, double step)
{
    if (std::abs(v) < step * 1e-9) v = 0;  // убрать "-0" и 1e-17
    double a = std::abs(v);
    if (a != 0 && (a >= 1e5 || a < 1e-3))
        std::snprintf(buf, n, "%.2g", v);
    else {
        int dec = std::max(0, (int)-std::floor(std::log10(step)));
        std::snprintf(buf, n, "%.*f", dec, v);
    }
}

} // namespace

void plotFunction(const std::vector<double>& X,
                  const std::vector<double>& Y,
                  const std::string& title)
{
    plotFunctions(X, {Y}, title);
}

void plotFunctions(const std::vector<double>& X,
                   const std::vector<std::vector<double>>& Ys,
                   const std::string& title)
{
    if (X.size() < 2 || Ys.empty()) {
        std::cerr << "plot: need at least 2 points and 1 curve\n";
        return;
    }
    for (const auto& Y : Ys)
        if (Y.size() != X.size()) {
            std::cerr << "plot: X and Y sizes differ\n";
            return;
        }

    // Диапазон данных (NaN/inf пропускаем).
    double xmin = std::numeric_limits<double>::infinity(), xmax = -xmin;
    double ymin = xmin, ymax = -xmin;
    for (double x : X)
        if (std::isfinite(x)) { xmin = std::min(xmin, x); xmax = std::max(xmax, x); }
    for (const auto& Y : Ys)
        for (double y : Y)
            if (std::isfinite(y)) { ymin = std::min(ymin, y); ymax = std::max(ymax, y); }
    if (!std::isfinite(xmin) || !std::isfinite(ymin)) {
        std::cerr << "plot: no finite data\n";
        return;
    }
    if (xmax == xmin) { xmin -= 1; xmax += 1; }
    if (ymax == ymin) { double d = std::max(1.0, std::abs(ymin)) * 0.5; ymin -= d; ymax += d; }
    double pad = 0.05 * (ymax - ymin);
    ymin -= pad; ymax += pad;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return;
    }
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer(title.c_str(), 1000, 650, SDL_WINDOW_RESIZABLE,
                                     &window, &renderer)) {
        std::cerr << "SDL window failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return;
    }
    SDL_SetRenderVSync(renderer, 1);

    std::vector<SDL_FPoint> pts(X.size());
    char buf[64];

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && (e.key.key == SDLK_ESCAPE || e.key.key == SDLK_Q))
                running = false;
        }

        int w, h;
        SDL_GetRenderOutputSize(renderer, &w, &h);
        const float left = 80, right = 20, top = 30, bottom = 40;
        const float pw = w - left - right, ph = h - top - bottom;
        if (pw < 10 || ph < 10) { SDL_Delay(16); continue; }

        auto sx = [&](double x) { return (float)(left + (x - xmin) / (xmax - xmin) * pw); };
        auto sy = [&](double y) { return (float)(top + (ymax - y) / (ymax - ymin) * ph); };

        // Фон
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Сетка и подписи делений
        double xs = niceStep(xmax - xmin, std::max(2, (int)(pw / 100)));
        double ys = niceStep(ymax - ymin, std::max(2, (int)(ph / 60)));

        for (double gx = std::ceil(xmin / xs) * xs; gx <= xmax + xs * 1e-9; gx += xs) {
            float px = sx(gx);
            SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
            SDL_RenderLine(renderer, px, top, px, top + ph);
            formatTick(buf, sizeof buf, gx, xs);
            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            SDL_RenderDebugText(renderer, px - 4.0f * SDL_strlen(buf), top + ph + 8, buf);
        }
        for (double gy = std::ceil(ymin / ys) * ys; gy <= ymax + ys * 1e-9; gy += ys) {
            float py = sy(gy);
            SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
            SDL_RenderLine(renderer, left, py, left + pw, py);
            formatTick(buf, sizeof buf, gy, ys);
            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            SDL_RenderDebugText(renderer, left - 8 - 8.0f * SDL_strlen(buf), py - 4, buf);
        }

        // Оси x=0, y=0, если попадают в область
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        if (ymin < 0 && ymax > 0) SDL_RenderLine(renderer, left, sy(0), left + pw, sy(0));
        if (xmin < 0 && xmax > 0) SDL_RenderLine(renderer, sx(0), top, sx(0), top + ph);

        // Рамка
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_FRect frame{left, top, pw, ph};
        SDL_RenderRect(renderer, &frame);

        // Кривые (толщина 2px: дублируем со сдвигом на 1 пиксель по y)
        SDL_Rect clip{(int)left, (int)top, (int)pw + 1, (int)ph + 1};
        SDL_SetRenderClipRect(renderer, &clip);
        for (size_t k = 0; k < Ys.size(); ++k) {
            const Color c = kPalette[k % std::size(kPalette)];
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
            const auto& Y = Ys[k];
            // Рисуем отрезками между NaN, чтобы разрывы не соединялись линией
            size_t start = 0;
            while (start < X.size()) {
                size_t n = 0;
                size_t i = start;
                for (; i < X.size() && std::isfinite(X[i]) && std::isfinite(Y[i]); ++i)
                    pts[n++] = {sx(X[i]), sy(Y[i])};
                if (n >= 2) {
                    SDL_RenderLines(renderer, pts.data(), (int)n);
                    for (size_t j = 0; j < n; ++j) pts[j].y += 1;
                    SDL_RenderLines(renderer, pts.data(), (int)n);
                } else if (n == 1) {
                    SDL_RenderPoint(renderer, pts[0].x, pts[0].y);
                }
                start = i + 1;
            }
        }
        SDL_SetRenderClipRect(renderer, nullptr);

        // Заголовок
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDebugText(renderer, left, 10, title.c_str());

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
