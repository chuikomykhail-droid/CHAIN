#pragma once
#include <vector>
#include <array>

std::vector<double> solve_tridiagonals(const std::vector<double>& A, std::vector<double> B,
                                       const std::vector<double>& C, std::vector<double> F);
std::vector<double> solveDE2(int N, double h, double BC0[3], double BCN[3],
                             const std::array<std::vector<double>, 4>& Cfs);
