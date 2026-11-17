#include "tridiagonals_solver.h"


/**
 * Метод прогонки
 * n - число уравнений (строк матрицы)
 * C - диагональ, лежащая над главной (нумеруется: [0;n-2])
 * B - главная диагональ матрицы A (нумеруется: [0;n-1])
 * A - диагональ, лежащая под главной (нумеруется: [1;n-1])
 * F - правая часть (столбец)
 * res - решение, массив x будет содержать ответ
 */
void solve(int n, double * A, double * B, double * C, double * F, double *res)
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