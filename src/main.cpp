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
	constexpr int N = 75;

    chainDemo(N, 1.0, 2.0);

	return 0;
}

void chainDemo(int N, double Mass, double Length)
{
    constexpr int subframes = 32;
    constexpr double K=1; // sqrt(K)

    double A[2] = {0, 0};
    double B[2] = {1.2, 0.5};

    PlotWindow win("CHACHA", -0.5, 2.5, -1.5, 1.5);
    win.addPoint({A[0], A[1]});
    int Bp = win.addPoint({B[0], B[1]}, true);
    Point prevB = win.point(Bp);

    Chain chain(Length, N, A, B, Mass);

    double v0[2]={0,0};
    double v[2]={0,0};
    double actualB0[2]={B[0], B[1]};
    double actualB[2]={B[0], B[1]};

    while (win.processEvents()) 
    {
        double dt = win.dt();
        if (dt > 0) 
        {                                  
            Point b = win.point(Bp);
    
            for (int i = 1; i<subframes+1; i++)
            {
                v[0]=-K*(actualB0[0]-b.x)*sin(K*(i*dt/subframes))+v0[0]*cos(K*(i*dt/subframes));
                v[1]=-K*(actualB0[1]-b.y)*sin(K*(i*dt/subframes))+v0[1]*cos(K*(i*dt/subframes));
                actualB[0] += v[0]*dt;
                actualB[1] += v[1]*dt;

                double dB[2] = {v[0]*dt, v[1]*dt};
                chain.Update(dB, std::min(dt/subframes, 1.0/30));

               
            }

            v0[0]=v[0];
            v0[1]=v[1];
            actualB0[0]=actualB[0];
            actualB0[1]=actualB[1];
        }
        
        win.setCurve(0, chain.X.F, chain.Y.F, 4.0f);
        win.render();
    }
}
