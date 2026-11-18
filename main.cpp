#include <iostream>
#include "DE2solver.h"
#include "plot.h"
#include "plotwindow.h"
#include <cmath>
#include <numbers>
#include <vector>

double p2(double x);
double p1(double x);
double p0(double x);
double B(double x);
void interactiveDemo(int N);
void DrawWithGivenBC(int N, double A[2], double B[2], std::vector<double>& X, std::vector<double>& Y);

int main() 
{
	constexpr int N = 9999;

    interactiveDemo(N);

	return 0;
}

double p2(double x)
{
    return 2*x*x;
}

double p1(double x)
{
    return -x;
}

double p0(double x)
{
    return -2;
}

double B0(double x)
{
    return 0;
}

// Интерактивное окно: A закреплена в (0,0), B можно таскать мышью
void interactiveDemo(int N)
{
    PlotWindow win("drag the orange point", -0.5, 2.5, -0.5, 2.5);
    int A  = win.addPoint({0, 0});
    int Bp = win.addPoint({1, 1}, true);

    std::vector<double> X(N+1);
    std::vector<double> Y(N+1);

    while (win.processEvents()) {
        if (win.pointsChanged()) {
            Point a = win.point(A), b = win.point(Bp);

            // TODO: здесь твоя парабола через a и b. Пока заглушка: отрезок.
            DrawWithGivenBC(N, new double[]{a.x, a.y}, new double[]{b.x, b.y}, X, Y);
            //std::cout << Y[4000];

            win.setCurve(0, X, Y);
        }
        win.render();
    }
}

void DrawWithGivenBC(int N, double A[2], double B[2], std::vector<double>& X, std::vector<double>& Y)
{
    double x0=A[0];
    double xN=B[0];

    double h = (xN-x0)/N;

    double *BC0 = new double[]{1, 0, A[1]};
    double *BCN = new double[]{1, 0, B[1]};

    std::array<std::vector<double>, 4> Cfs;
    for (int i = 0; i<4; i++){
        Cfs[i].reserve(N+1);
    }

    for (int i=0; i<N+1; i++)
    {
        double xi=x0+i*h;

        Cfs[2].push_back(p2(xi));
        Cfs[1].push_back(p1(xi));
        Cfs[0].push_back(p0(xi));
        Cfs[3].push_back(B0(xi) );
    }

    std::vector<double> y = solveDE2(N, h,BC0, BCN, Cfs);

    for (int i = 0; i < N+1; i++) {
        X[i] = x0 + i*h;
        Y[i] = y[i];
    }
}
