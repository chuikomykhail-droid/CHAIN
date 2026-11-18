#pragma once
#include <vector>

class GridFunction
{
public:
    std::vector<double> X, F, D1, D2, D3;

    GridFunction() = default;   // пустой объект: потом присвоить GridFunction(X, F, N, maxDegree)
    GridFunction(const std::vector<double>& X, const std::vector<double>& F, int N, int maxDegree);

    /*
    Next step by time
    */
    void Integrate(const std::vector<double>& dXdt, const std::vector<double>& dXdt2);

    // Производная порядка order (1, 2 или 3) функции F на равномерной сетке с шагом h.
    // Второй порядок точности везде, включая края. Нужно F.size() >= order + 2.
    // Вызов: GridFunction::Derivative(F, h, 1)
    static std::vector<double> Derivative(const std::vector<double>& F, double h, int order);

private:
    int N = 0, maxDegree = 0;
    double h = 0;

    void CalculateDerivatives();
};
