#include "gridFunction.h"

GridFunction::GridFunction(const std::vector<double>& X, const std::vector<double>& F, int N, int maxDegree)
{
    this->X = X;
    this->F = F;
    this->maxDegree=maxDegree;
    this->N=N;

    h=(X[N]-X[0])/N;

    CalculateDerivatives();
}

void GridFunction::Integrate(const std::vector<double>& dXdt, const std::vector<double>& dXdt2)
{
    for (int i=0; i<N+1; i++)
    {
        X[i] += dXdt[i]*h + dXdt2[i]*h*h/2;
    }

    CalculateDerivatives();
}

void GridFunction::CalculateDerivatives()
{
    if (maxDegree < 1){
        return;
    }

    D1.resize(N+1);
    D1[0]=(-3*F[0]+4*F[1]-F[2])/(2*h);
    for (int i=0; i<N; i++)
    {
        D1[i]=(F[i+1]-F[i-1])/(2*h);
    }
    D1[N]=(3*F[N]-4*F[N-1]+F[N-2])/(2*h);

    if (maxDegree < 2){
        return;
    }

    D2.resize(N+1);
    D2[0] = (2*F[0] - 5*F[1] + 4*F[2] - F[3]) / (h*h);
    for (int i=1; i<N; i++)
    {
        D2[i] = (F[i+1] - 2*F[i] + F[i-1]) / (h*h);
    }
    D2[N] = (2*F[N] - 5*F[N-1] + 4*F[N-2] - F[N-3]) / (h*h);

    if (maxDegree < 3){
        return;
    }

    D3.resize(N+1);
    const double h3 = 2*h*h*h;
    D3[0]   = (-5*F[0] + 18*F[1] - 24*F[2] + 14*F[3] - 3*F[4]) / h3;
    D3[1]   = (-3*F[0] + 10*F[1] - 12*F[2] +  6*F[3] -   F[4]) / h3;
    for (int i=2; i<N-1; i++)
    {
        D3[i] = (F[i+2] - 2*F[i+1] + 2*F[i-1] - F[i-2]) / h3;
    }
    D3[N-1] = ( 3*F[N] - 10*F[N-1] + 12*F[N-2] -  6*F[N-3] +   F[N-4]) / h3;
    D3[N]   = ( 5*F[N] - 18*F[N-1] + 24*F[N-2] - 14*F[N-3] + 3*F[N-4]) / h3;
}
