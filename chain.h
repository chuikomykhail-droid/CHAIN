#pragma once
#include <vector>
#include "gridFunction.h"

class Chain
{
public:
    // Положение нити: X.F[i], Y.F[i] — координаты точки с параметром l_i = i*h.
    // X.F[0], Y.F[0] — точка A, X.F[N], Y.F[N] — точка B.
    GridFunction X, Y;

    Chain(double Length, int N, double A[2], double B[2], double M);

    // Шаг по времени dt; dB — сдвиг точки B за этот шаг.
    void Update(double dB[2], double dt);

private:
    std::vector<double> L;
    GridFunction Xt, Yt;

    int N;
    double h;
    double M, Length;

    /*
    Заполняет X, Y цепной линией длины Length через A и B
    по равномерной сетке по длине дуги: l_i = i*h, h = Length/N.
    Требуется A[0] != B[0] и Length > |AB|.

    Цепная линия y = a*cosh((x - x0)/a) + c.
    По длине дуги s от нижней точки: x(s) = x0 + a*asinh(s/a),  y(s) = c + sqrt(a^2 + s^2).
    */
    void CalculateCatenary(double Length, double A[2], double B[2]);
};
