#include <iostream>
#include "DE2solver.h"
#include <cmath>
#include <numbers>
#include <vector>

double p2(double x);
double p1(double x);
double p0(double x);
double B(double x);

int main() 
{
    // Пример
	constexpr int N = 9999;
    
	double *y = new double[N+1];

    double x0=0;
    double xN=std::numbers::pi;

    double h = (xN-x0)/N;

    double *BC0 = new double[]{1, -1, 1};
    double *BCN = new double[]{1, -1, -1};

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
        Cfs[3].push_back(B(xi) );
    }

    solveDE2(N, h,BC0, BCN, Cfs, y);
    
    

	double av = 0;
    for (int i = 0; i < N+1; i++) 
    {
		av += (y[i]-cos(x0+i*h))/(N+1);
	}

    std::cout << std::endl << av;

    //std::cout << y[0];

	return 0;
}

double p2(double x)
{
    return 1;
}

double p1(double x)
{
    return 1;
}

double p0(double x)
{
    return 1;
}

double B(double x)
{
    return sin(x);
}
