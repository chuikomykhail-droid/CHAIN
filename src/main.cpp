#include <iostream>
#include "DE2solver.h"
#include "plot.h"
#include "plotwindow.h"
#include "chain.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

void chainDemo(int N, double Mass, double Length);

int main() 
{
	constexpr int N = 30;

    chainDemo(N, 1.0, 2.5);

	return 0;
}

void chainDemo(int N, double Mass, double Length)
{
    double A[2] = {0, 0};
    double B[2] = {1, 1};
    Chain chain(Length, N, A, B, Mass);

    PlotWindow win("CHACHA", -0.5, 2.5, -1.5, 1.5);
    win.addPoint({A[0], A[1]});
    int Bp = win.addPoint({B[0], B[1]}, true);
    Point prevB = win.point(Bp);

    while (win.processEvents()) {
        double dt = win.dt();
        if (dt > 0) {                                  
            Point b = win.point(Bp);
            double dB[2] = {b.x - prevB.x, b.y - prevB.y};
            prevB = b;

            chain.Update(dB, std::min(dt, 1.0/30));   
        }
        win.setCurve(0, chain.X.F, chain.Y.F, 4.0f);
        win.render();
    }
}
