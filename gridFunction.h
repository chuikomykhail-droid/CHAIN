#pragma once
#include <vector>

class GridFunction
{
public:
    std::vector<double> X, F, D1, D2, D3;

    GridFunction(const std::vector<double>& X, const std::vector<double>& F, int N, int maxDegree);

    /*
    Next step by time
    */
    void Integrate(const std::vector<double>& dXdt, const std::vector<double>& dXdt2);

private:
    int N, maxDegree;
    double h;

    void CalculateDerivatives();
};
