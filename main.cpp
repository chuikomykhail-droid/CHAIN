#include <iostream>
#include "tridiagonals_solver.h"
#include <cmath>
#include <numbers>
#include <chrono>

double p2(double x);
double p1(double x);
double p0(double x);
double B(double x);

int main() 
{
    auto start = std::chrono::steady_clock::now();

    // Пример
	int N = 100000;
    
	double *b = new double[N];
	double *c = new double[N];
	double *a = new double[N];
	double *x = new double[N];
	double *f = new double[N];

    double x0=0;
    double xN=std::numbers::pi;
    double K0=1;
    double KN=-1;
    double h=(xN-x0)/N;

    f[0]=K0;
    f[N]=KN;

	for (int i = 0; i < N+1; i++) 
    {
		double xi=x0+i*h;

        c[i] = (i == 0) ? 0 : p1(xi)/h + p2(xi)/(h*h);
        a[i] = (i==N) ? 0 : p2(xi)/(h*h);
        b[i] = (i==0 || i==N) ?  1 : -(p1(xi)/h + 2*p2(xi)/(h*h)) + p0(xi);

        if (0<i && i < N)
        {
            f[i]=B(xi);
        }
	}

    // for (int i = 0; i < N+1; i++)
    // {
    //     std::cout << "i:" << i << "F:" << f[i] << "c:" << c[i] << "B:" << b[i] << std::endl;
    // }

	solve(N+1, a, b, c, f, x);

    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout <<  ms.count() << "FF";

	// for (int i = 0; i < N+1; i++) 
    // {
	// 	std::cout << x[i]-cos(x0+i*h) << std::endl;
	// }

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
    return -sin(x);
}
