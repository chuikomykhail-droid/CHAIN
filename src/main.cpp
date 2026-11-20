#include <iostream>
#include "DE2solver.h"
#include "plot.h"
#include "plotwindow.h"
#include "chain.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

void chainDemo(int N);

int main() 
{
	constexpr int N = 50;

    chainDemo(N);

	return 0;
}

void chainDemo(int N)
{
    double A[2] = {0, 0}, B[2] = {1, 1};
    double Length = 2.0, M = 1.0;
    Chain chain(Length, N, A, B, M);

    PlotWindow win("chain: drag the orange point", -0.5, 2.5, -1.5, 1.5);
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
        win.setCurve(0, chain.X.F, chain.Y.F);
        win.render();
    }
}
