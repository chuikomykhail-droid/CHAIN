#include "DE2solver.h"


/**
 * Метод прогонки
 * n - число уравнений (строк матрицы)
 * C - диагональ, лежащая над главной (нумеруется: [0;n-2])
 * B - главная диагональ матрицы A (нумеруется: [0;n-1])
 * A - диагональ, лежащая под главной (нумеруется: [1;n-1])
 * F - правая часть (столбец)
 * res - решение, массив x будет содержать ответ
 */
void solve_tridiagonals(int n, double * A, double * B, double * C, double * F, double *res)
{
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
}

/**
* N - steps number
* x0 - left end
* XN - right end
* BC0 - Boundary condition in x0: BC[0]f + BC[1]f' = BC[2]
* BCN - Boundary condition in xN: BC[0]f + BC[1]f' = BC[2]
* Coofs -- cooficients of DE : Cfs[2]f'' + Cfs[1]f' + Cfs[0]f + Cfs[3] = 0
*/
void solveDE2(int N, double h, double *BC0, double *BCN, double  Cfs[4][10000], double *res)
{
	double *a = new double[N+1];
	double *b = new double[N+1];
	double *c = new double[N+1];
	double *f = new double [N+1];

	b[0]=BC0[0]-BC0[1]/h;
	c[0]=BC0[1]/h;
	f[0]=BC0[2];

	a[N]=-BCN[1]/h;
	b[N]=BCN[0]+BCN[1]/h;
	f[N]=BCN[2];

	for (int i = 1; i < N; i++) 
    {
        c[i] =  Cfs[1][i]/h + Cfs[2][i]/(h*h);
        a[i] =  Cfs[2][i]/(h*h);
        b[i] = -(Cfs[1][i]/h + 2*Cfs[2][i]/(h*h)) + Cfs[0][i];

        f[i]=-Cfs[3][i];
	}

	solve_tridiagonals(N+1,a,b,c,f,res);
}