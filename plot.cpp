#include "plot.h"
#include "plotwindow.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

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

    // Диапазон осей по данным (NaN/inf пропускаем).
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

    try {
        PlotWindow win(title, xmin, xmax, ymin, ymax);
        for (size_t k = 0; k < Ys.size(); ++k)
            win.setCurve((int)k, X, Ys[k]);
        while (win.processEvents())
            win.render();
    } catch (const std::exception& e) {
        std::cerr << "plot: " << e.what() << "\n";
    }
}
