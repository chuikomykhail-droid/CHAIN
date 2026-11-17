#include <iostream>
#include "tridiagonals_solver.h"
#include <cmath>

int main() 
{
    // Пример
	int n = 3;
	double *b = new double[n];
	double *c = new double[n];
	double *a = new double[n];
	double *x = new double[n];
	double *f = new double[n];

	for (int i = 0; i < n; i++) {
		c[i] = -2;
	}

	for (int i = 0; i < n; i++) {
		a[i] = -2;
	}

	f[0] = -2;
	f[1] = -6;
	f[2] = -2;

	for (int i = 0; i < n; i++) {
		b[i] = 1;
	}

	solve(n, a, b, c, f);

	for (int i = 0; i < n; i++) {
		std::cout << x[i] << std::endl;
	}

	return 0;
}
