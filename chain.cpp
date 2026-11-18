#include<vector>
#include <algorithm>
#include <cmath>
#include <array>
#include "gridFunction.h"
#include "DE2solver.h"

class Chain
{
private:
    std::vector<double> L;
    GridFunction Xt, Yt;

    int N;
    double h;
    double M, Length;

    /*
    Заполняет X, Y цепной линией длины Length через A и B
    по равномерной сетке по длине дуги: l_i = i*h, h = Length/N.
    X.F[0], Y.F[0] — точка A, X.F[N], Y.F[N] — точка B.
    Требуется A[0] != B[0] и Length > |AB|, иначе std::invalid_argument.

    Цепная линия y = a*cosh((x - x0)/a) + c.
    По длине дуги s от нижней точки: x(s) = x0 + a*asinh(s/a),  y(s) = c + sqrt(a^2 + s^2).
    */
    void CalculateCatenary(double Length, double A[2], double B[2])
    {
        // считаем для x1 < x2; если A правее B, в конце развернём массивы
        bool swapped = B[0] < A[0];
        double x1 = swapped ? B[0] : A[0], y1 = swapped ? B[1] : A[1];
        double x2 = swapped ? A[0] : B[0], y2 = swapped ? A[1] : B[1];

        double dx = x2 - x1, dy = y2 - y1;

        // 1) параметр a: sinh(xi)/xi = r,  xi = dx/(2a)
        double r = std::sqrt(Length*Length - dy*dy) / dx;   // > 1
        double lo = 0, hi = 1;
        while (std::sinh(hi) / hi < r) hi *= 2;
        for (int it = 0; it < 200; it++)                    // деление пополам
        {
            double mid = 0.5 * (lo + hi);
            if (std::sinh(mid) / mid < r) lo = mid; else hi = mid;
        }
        double xi = 0.5 * (lo + hi);
        double a = dx / (2 * xi);

        // 2) абсцисса минимума и сдвиг по высоте
        double x0 = 0.5 * (x1 + x2) - a * std::atanh(dy / Length);
        double c  = y1 - a * std::cosh((x1 - x0) / a);

        // 3) точки по равномерной сетке по длине дуги
        double s1 = a * std::sinh((x1 - x0) / a);           // длина дуги в левой точке

        std::vector<double> xs(N+1), ys(N+1);
        for (int i = 0; i <= N; i++)
        {
            double s = s1 + i * h;
            xs[i] = x0 + a * std::asinh(s / a);
            ys[i] = c + std::sqrt(a*a + s*s);
        }

        // чтобы l = 0 всегда соответствовало точке A
        if (swapped)
        {
            std::reverse(xs.begin(), xs.end());
            std::reverse(ys.begin(), ys.end());
        }

        X = GridFunction(L, xs, N, 3);
        Y = GridFunction(L, ys, N, 3);
    }

public:
    GridFunction X, Y;

    Chain(double Length, int N, double A[2], double B[2], double M)
    {
        this->N = N;
        this->M = M;
        this->Length = Length;
        h = Length / N;
        L.resize(N+1);
        for (int i=0; i<N+1; i++)
        {
            L[i]=i*h;
        }

        Xt = GridFunction(L, std::vector<double>(N+1, 0.0), N, 3);
        Yt = GridFunction(L, std::vector<double>(N+1, 0.0), N, 3);

        CalculateCatenary(Length, A, B);
    }

    void Update(double dB[2], double dt)
    {
        double vxN_old=Xt.F[N];
        double vyN_old=Yt.F[N];

        double vxN=dB[0]/dt;
        double vyN=dB[1]/dt;

        double axN = (vxN-vxN_old)/dt;
        double ayN = (vyN-vyN_old)/dt;

        std::array<std::vector<double>, 4> Cfs;
        for (int i = 0; i<4; i++){
            Cfs[i].resize(N+1);
        }

        for (int i= 0; i<N+1; i++)
        {
            Cfs[2][i]=X.D1[i]*X.D1[i] + Y.D1[i]*Y.D1[i];
            Cfs[1][i]=2*Y.D1[i]*(Y.D2[i]-(Y.D1[i]*X.D2[i])/X.D1[i]);
            Cfs[0][i]=Y.D1[i]*(Y.D3[i]-(2*Y.D2[i]*X.D2[i]+Y.D1[i]*X.D3[i])/X.D1[i] + 2*Y.D1[i]*X.D2[i]*X.D2[i]/(X.D1[i]*X.D1[i]) );
            Cfs[3][i]=(M/Length)*X.D1[i]*(Xt.D1[i]*Xt.D1[i] + Yt.D1[i]*Yt.D1[i]);
        }

        double *BC0 = new double[3]{1,0,0};
        double alpha= Y.D2[N]*(1/X.D1[N]) - Y.D1[N]*pow(X.D1[N], -1)*X.D2[N];
        double beta = Y.D1[N]*pow(X.D1[N], -1);
        double gamma = (M/Length)*(9.807+ayN);

        double *BCN = new double[3]{alpha, beta, gamma};

        std::vector<double> H = solveDE2(N, h, BC0, BCN, Cfs);
        std::vector<double> Hl = GridFunction::Derivative(H, h, 1);


    }
};
