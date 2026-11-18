#pragma once
#include <iostream>

#ifndef TRIDIAGONALS_SOLVER
#define TRIDIAGONALS_SOLVER

void solve_tridiagonals(int n, double *A, double *B, double *C, double *F, double *res);
void solveDE2(int N, double h, double *BC0, double *BCN, double Cfs[4][10000], double *res);

#endif