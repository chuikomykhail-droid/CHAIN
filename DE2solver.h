#pragma once
#include <vector>
#include <array>

#ifndef TRIDIAGONALS_SOLVER
#define TRIDIAGONALS_SOLVER

void solve_tridiagonals(int n, double *A, double *B, double *C, double *F, double *res);
void solveDE2(int N, double h, double BC0[3], double BCN[3], std::array<std::vector<double>, 4>& arr, double *res);

#endif