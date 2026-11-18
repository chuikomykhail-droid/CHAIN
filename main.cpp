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

    double Cfs[4][10000]{};

    std:: cout << "ALLO";

    for (int i=0; i<N+1; i++)
    {
        //std:: cout << "ALLO" << i;
        double xi=x0+i*h;
        Cfs[2][i]=p2(xi);
        Cfs[1][i]=p1(xi);
        Cfs[0][i]=p0(xi);
        Cfs[3][i]=B(xi);
    }

    solveDE2(N, h,BC0, BCN, Cfs, y);
    
    

	for (int i = 0; i < N+1; i++) 
    {
		std::cout << y[i]-cos(x0+i*h) << std::endl;
	}

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
