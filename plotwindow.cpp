#include "plotwindow.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Пересчёт координат: мир (x, y) <-> экран (пиксели). Всё в одном месте.
// ---------------------------------------------------------------------------
struct PlotWindow::Viewport {
    float left, top, w, h;              // область графика на экране
    double xmin, xmax, ymin, ymax;      // что в неё попадает

    float  sx(double x) const { return (float)(left + (x - xmin) / (xmax - xmin) * w); }
    float  sy(double y) const { return (float)(top + (ymax - y) / (ymax - ymin) * h); }
    double wx(float px) const { return xmin + (px - left) / w * (xmax - xmin); }
    double wy(float py) const { return ymax - (py - top) / h * (ymax - ymin); }
    bool   valid() const { return w > 10 && h > 10; }
};

namespace {

struct Color { Uint8 r, g, b; };

const Color kPalette[] = {
    {31, 119, 180}, {214, 39, 40}, {44, 160, 44},
    {255, 127, 14}, {148, 103, 189}, {140, 86, 75},
};

constexpr float kMarginLeft = 80, kMarginRight = 20, kMarginTop = 30, kMarginBottom = 40;
constexpr float kPointRadius = 6;
constexpr float kHitRadius   = 12;   // радиус "захвата" точки мышью, в пикселях

// "Красивый" шаг сетки: 1, 2 или 5 * 10^k.
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

void fillCircle(SDL_Renderer* r, float cx, float cy, float rad, Color c)
{
    constexpr int N = 24;
    SDL_Vertex v[N + 1];
    int idx[N * 3];
    SDL_FColor fc{c.r / 255.f, c.g / 255.f, c.b / 255.f, 1.f};
    v[0] = {{cx, cy}, fc, {0, 0}};
    for (int i = 0; i < N; ++i) {
        float a = 2 * SDL_PI_F * i / N;
        v[i + 1] = {{cx + rad * SDL_cosf(a), cy + rad * SDL_sinf(a)}, fc, {0, 0}};
        idx[3 * i] = 0;
        idx[3 * i + 1] = i + 1;
        idx[3 * i + 2] = (i + 1) % N + 1;
    }
    SDL_RenderGeometry(r, nullptr, v, N + 1, idx, N * 3);
}

} // namespace

// ---------------------------------------------------------------------------

PlotWindow::PlotWindow(const std::string& title,
                       double xmin, double xmax, double ymin, double ymax,
                       int width, int height)
    : title_(title)
{
    setRange(xmin, xmax, ymin, ymax);

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());

    if (!SDL_CreateWindowAndRenderer(title.c_str(), width, height, SDL_WINDOW_RESIZABLE,
                                     &window_, &renderer_)) {
        std::string err = SDL_GetError();
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw std::runtime_error("SDL window failed: " + err);
    }
    SDL_SetRenderVSync(renderer_, 1);
    cursorHand_    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    cursorDefault_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
}

PlotWindow::~PlotWindow()
{
    if (cursorHand_)    SDL_DestroyCursor(cursorHand_);
    if (cursorDefault_) SDL_DestroyCursor(cursorDefault_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void PlotWindow::setRange(double xmin, double xmax, double ymin, double ymax)
{
    if (!(xmax > xmin) || !(ymax > ymin))
        throw std::invalid_argument("PlotWindow::setRange: need xmax > xmin and ymax > ymin");
    xmin_ = xmin; xmax_ = xmax; ymin_ = ymin; ymax_ = ymax;
}

int PlotWindow::addPoint(Point p, bool draggable)
{
    points_.push_back({p, draggable});
    changed_ = true;
    return (int)points_.size() - 1;
}

Point PlotWindow::point(int id) const { return points_.at(id).p; }

void PlotWindow::setPoint(int id, Point p)
{
    points_.at(id).p = p;
    changed_ = true;
}

bool PlotWindow::pointsChanged()
{
    bool c = changed_;
    changed_ = false;
    return c;
}

void PlotWindow::setCurve(int index, const std::vector<double>& X, const std::vector<double>& Y)
{
    if (X.size() != Y.size())
        throw std::invalid_argument("PlotWindow::setCurve: X and Y sizes differ");
    if (index < 0) throw std::invalid_argument("PlotWindow::setCurve: negative index");
    if ((size_t)index >= curves_.size()) curves_.resize(index + 1);
    curves_[index].X = X;
    curves_[index].Y = Y;
}

void PlotWindow::clearCurves() { curves_.clear(); }

PlotWindow::Viewport PlotWindow::viewport() const
{
    int w = 0, h = 0;
    SDL_GetRenderOutputSize(renderer_, &w, &h);
    return {kMarginLeft, kMarginTop,
            w - kMarginLeft - kMarginRight, h - kMarginTop - kMarginBottom,
            xmin_, xmax_, ymin_, ymax_};
}

int PlotWindow::hitTest(const Viewport& vp, float px, float py) const
{
    int best = -1;
    float bestD2 = kHitRadius * kHitRadius;
    for (size_t i = 0; i < points_.size(); ++i) {
        if (!points_[i].draggable) continue;
        float dx = vp.sx(points_[i].p.x) - px;
        float dy = vp.sy(points_[i].p.y) - py;
        float d2 = dx * dx + dy * dy;
        if (d2 <= bestD2) { bestD2 = d2; best = (int)i; }
    }
    return best;
}

// ---------------------------------------------------------------------------
// События
// ---------------------------------------------------------------------------
bool PlotWindow::processEvents()
{
    const Viewport vp = viewport();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        // координаты мыши -> координаты рендера (важно при масштабировании экрана в Windows)
        SDL_ConvertEventToRenderCoordinates(renderer_, &e);

        switch (e.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            return false;

        case SDL_EVENT_KEY_DOWN:
            if (e.key.key == SDLK_ESCAPE || e.key.key == SDLK_Q) return false;
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (e.button.button == SDL_BUTTON_LEFT && vp.valid())
                dragged_ = hitTest(vp, e.button.x, e.button.y);
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (e.button.button == SDL_BUTTON_LEFT) dragged_ = -1;
            break;

        case SDL_EVENT_MOUSE_MOTION:
            if (!vp.valid()) break;
            if (dragged_ >= 0) {
                // не даём утащить точку за пределы осей
                double x = std::clamp(vp.wx(e.motion.x), xmin_, xmax_);
                double y = std::clamp(vp.wy(e.motion.y), ymin_, ymax_);
                points_[dragged_].p = {x, y};
                changed_ = true;
            } else {
                int h = hitTest(vp, e.motion.x, e.motion.y);
                if (h != hovered_) {
                    hovered_ = h;
                    SDL_SetCursor(h >= 0 ? cursorHand_ : cursorDefault_);
                }
            }
            break;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Отрисовка
// ---------------------------------------------------------------------------
void PlotWindow::render()
{
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderClear(renderer_);

    const Viewport vp = viewport();
    if (vp.valid()) {
        drawGrid(vp);

        SDL_Rect clip{(int)vp.left, (int)vp.top, (int)vp.w + 1, (int)vp.h + 1};
        SDL_SetRenderClipRect(renderer_, &clip);
        drawCurves(vp);
        SDL_SetRenderClipRect(renderer_, nullptr);

        drawPoints(vp);
    }

    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderDebugText(renderer_, kMarginLeft, 10, title_.c_str());

    SDL_RenderPresent(renderer_);  // с vsync ждёт следующего кадра
}

void PlotWindow::drawGrid(const Viewport& vp)
{
    char buf[64];
    const float left = vp.left, top = vp.top, pw = vp.w, ph = vp.h;
    double xs = niceStep(xmax_ - xmin_, std::max(2, (int)(pw / 100)));
    double ys = niceStep(ymax_ - ymin_, std::max(2, (int)(ph / 60)));

    for (double gx = std::ceil(xmin_ / xs) * xs; gx <= xmax_ + xs * 1e-9; gx += xs) {
        float px = vp.sx(gx);
        SDL_SetRenderDrawColor(renderer_, 230, 230, 230, 255);
        SDL_RenderLine(renderer_, px, top, px, top + ph);
        formatTick(buf, sizeof buf, gx, xs);
        SDL_SetRenderDrawColor(renderer_, 60, 60, 60, 255);
        SDL_RenderDebugText(renderer_, px - 4.0f * SDL_strlen(buf), top + ph + 8, buf);
    }
    for (double gy = std::ceil(ymin_ / ys) * ys; gy <= ymax_ + ys * 1e-9; gy += ys) {
        float py = vp.sy(gy);
        SDL_SetRenderDrawColor(renderer_, 230, 230, 230, 255);
        SDL_RenderLine(renderer_, left, py, left + pw, py);
        formatTick(buf, sizeof buf, gy, ys);
        SDL_SetRenderDrawColor(renderer_, 60, 60, 60, 255);
        SDL_RenderDebugText(renderer_, left - 8 - 8.0f * SDL_strlen(buf), py - 4, buf);
    }

    // оси x=0, y=0
    SDL_SetRenderDrawColor(renderer_, 150, 150, 150, 255);
    if (ymin_ < 0 && ymax_ > 0) SDL_RenderLine(renderer_, left, vp.sy(0), left + pw, vp.sy(0));
    if (xmin_ < 0 && xmax_ > 0) SDL_RenderLine(renderer_, vp.sx(0), top, vp.sx(0), top + ph);

    // рамка
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_FRect frame{left, top, pw, ph};
    SDL_RenderRect(renderer_, &frame);
}

void PlotWindow::drawCurves(const Viewport& vp)
{
    for (size_t k = 0; k < curves_.size(); ++k) {
        const auto& X = curves_[k].X;
        const auto& Y = curves_[k].Y;
        const Color c = kPalette[k % std::size(kPalette)];
        SDL_SetRenderDrawColor(renderer_, c.r, c.g, c.b, 255);

        std::vector<SDL_FPoint> pts(X.size());

        // рисуем кусками между NaN/inf, чтобы разрывы не соединялись линией
        size_t start = 0;
        while (start < X.size()) {
            size_t n = 0, i = start;
            for (; i < X.size() && std::isfinite(X[i]) && std::isfinite(Y[i]); ++i)
                pts[n++] = {vp.sx(X[i]), vp.sy(Y[i])};
            if (n >= 2) {
                SDL_RenderLines(renderer_, pts.data(), (int)n);
                for (size_t j = 0; j < n; ++j) pts[j].y += 1;   // толщина 2px
                SDL_RenderLines(renderer_, pts.data(), (int)n);
            } else if (n == 1) {
                SDL_RenderPoint(renderer_, pts[0].x, pts[0].y);
            }
            start = i + 1;
        }
    }
}

void PlotWindow::drawPoints(const Viewport& vp)
{
    char buf[64];
    for (size_t i = 0; i < points_.size(); ++i) {
        const auto& h = points_[i];
        float px = vp.sx(h.p.x), py = vp.sy(h.p.y);
        bool active = ((int)i == dragged_ || (int)i == hovered_);

        if (h.draggable) {
            fillCircle(renderer_, px, py, kPointRadius + (active ? 3.f : 1.5f), {0, 0, 0});
            fillCircle(renderer_, px, py, kPointRadius + (active ? 1.5f : 0.f), {255, 140, 0});
        } else {
            fillCircle(renderer_, px, py, kPointRadius - 1, {40, 40, 40});
        }

        if (active) {  // подпись с координатами
            std::snprintf(buf, sizeof buf, "(%.3g, %.3g)", h.p.x, h.p.y);
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderDebugText(renderer_, px + 12, py - 16, buf);
        }
    }
}
