#include "DE2solver.h"


/**
 * Метод прогонки
 * n - число уравнений (строк матрицы) = B.size()
 * C - диагональ, лежащая над главной (нумеруется: [0;n-2])
 * B - главная диагональ матрицы A (нумеруется: [0;n-1])
 * A - диагональ, лежащая под главной (нумеруется: [1;n-1])
 * F - правая часть (столбец)
 * Возвращает решение. B и F принимаются по значению: алгоритм их портит.
 */
std::vector<double> solve_tridiagonals(const std::vector<double>& A, std::vector<double> B,
                                       const std::vector<double>& C, std::vector<double> F)
{
	int n = (int)B.size();
	std::vector<double> res(n);
	double temp;

	for (int i = 1; i < n; i++)
	{
		temp = A[i] / B[i - 1];
		B[i] = B[i] - temp * C[i - 1];
		F[i] = F[i] - temp * F[i - 1];
	}

	res[n - 1] = F[n - 1] / B[n - 1];

	for (int i = n - 2; i >= 0; i--)
	{
		res[i] = (F[i] - C[i] * res[i + 1]) / B[i];
	}

	return res;
}

/**
* N - steps number
* x0 - left end
* XN - right end
* BC0 - Boundary condition in x0: BC[0]f + BC[1]f' = BC[2]
* BCN - Boundary condition in xN: BC[0]f + BC[1]f' = BC[2]
* Coofs -- cooficients of DE : Cfs[2]f'' + Cfs[1]f' + Cfs[0]f + Cfs[3] = 0
* returns f on the grid, N+1 values
*/
std::vector<double> solveDE2(int N, double h, double BC0[3], double BCN[3], const std::array<std::vector<double>, 4>& Cfs)
{
	std::vector<double> a(N+1), b(N+1), c(N+1), f(N+1);

	if (BC0[1] != 0)
	{
		double P = Cfs[2][0]/(h*h), Q = Cfs[1][0]/(2*h), s = 2*h/BC0[1];
		b[0]=-2*P + (P-Q)*s*BC0[0] + Cfs[0][0];
		c[0]=2*P;
		f[0]=-Cfs[3][0] + (P-Q)*s*BC0[2];
	}
	else
	{
		b[0]=BC0[0];
		c[0]=0;
		f[0]=BC0[2];
	}

	if (BCN[1] != 0)
	{
		double P = Cfs[2][N]/(h*h), Q = Cfs[1][N]/(2*h), s = 2*h/BCN[1];
		a[N]=2*P;
		b[N]=-2*P - (P+Q)*s*BCN[0] + Cfs[0][N];
		f[N]=-Cfs[3][N] - (P+Q)*s*BCN[2];
	}
	else
	{
		a[N]=0;
		b[N]=BCN[0];
		f[N]=BCN[2];
	}

	for (int i = 1; i < N; i++)
    {
        c[i] =  Cfs[2][i]/(h*h) + Cfs[1][i]/(2*h);
        a[i] =  Cfs[2][i]/(h*h) - Cfs[1][i]/(2*h);
        b[i] = -2*Cfs[2][i]/(h*h) + Cfs[0][i];

        f[i]=-Cfs[3][i];
	}

	return solve_tridiagonals(a, std::move(b), c, std::move(f));
}