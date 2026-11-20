#pragma once
#include <string>
#include <vector>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Cursor;

struct Point { double x = 0, y = 0; };

// Неблокирующее окно с графиком. Цикл кадров крутится снаружи (в main):
//
//     PlotWindow win("title", xmin, xmax, ymin, ymax);
//     int B = win.addPoint({1, 1}, true);        // точка, которую можно таскать
//     while (win.processEvents()) {
//         if (win.pointsChanged()) {
//             Point b = win.point(B);
//             ... посчитать X, Y ...
//             win.setCurve(0, X, Y);
//         }
//         win.render();                         // ~60 кадров/с (vsync)
//     }
//
// Окно ничего не считает: только хранит точки и кривые и рисует их.
// Диапазон осей фиксирован (setRange), чтобы точка не "убегала" из-под мыши.
class PlotWindow {
public:
    PlotWindow(const std::string& title,
               double xmin, double xmax, double ymin, double ymax,
               int width = 1700, int height = 1300);
    ~PlotWindow();
    PlotWindow(const PlotWindow&) = delete;
    PlotWindow& operator=(const PlotWindow&) = delete;

    // Обработать события (мышь, клавиатура). false — окно закрыли (крестик / Esc / Q).
    bool processEvents();
    // Время в секундах между двумя последними вызовами processEvents() (длительность кадра).
    // При vsync обычно ~1/60 с. На первом кадре — 0.
    double dt() const { return dt_; }
    // Нарисовать кадр. Ждёт vsync, поэтому задаёт частоту цикла.
    void render();

    void setRange(double xmin, double xmax, double ymin, double ymax);

    // Точки. Возвращает id точки (0, 1, 2, ...).
    int   addPoint(Point p, bool draggable = false);
    Point point(int id) const;
    void  setPoint(int id, Point p);
    // true, если какую-то точку сдвинули с прошлого вызова (первый вызов — всегда true).
    bool  pointsChanged();

    // Кривые. index — номер кривой (0, 1, ...), цвет зависит от номера.
    // width — толщина линии в пикселях.
    void setCurve(int index, const std::vector<double>& X, const std::vector<double>& Y,
                  float width = 2.0f);
    void clearCurves();

private:
    struct Viewport;
    struct Curve  { std::vector<double> X, Y; float width = 2.0f; };
    struct Handle { Point p; bool draggable; };

    Viewport viewport() const;
    int  hitTest(const Viewport& vp, float px, float py) const;
    void drawGrid(const Viewport& vp);
    void drawCurves(const Viewport& vp);
    void drawPoints(const Viewport& vp);

    SDL_Window*   window_   = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Cursor*   cursorHand_    = nullptr;
    SDL_Cursor*   cursorDefault_ = nullptr;

    std::string title_;
    double xmin_, xmax_, ymin_, ymax_;

    std::vector<Curve>  curves_;
    std::vector<Handle> points_;
    int  dragged_ = -1;   // id точки, которую сейчас тащат
    int  hovered_ = -1;   // id точки под курсором
    bool changed_ = true;

    unsigned long long lastTicksNS_ = 0;   // время прошлого processEvents(), нс
    double dt_ = 0;
};
