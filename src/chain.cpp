#include "chain.h"
#include <algorithm>
#include <cmath>
#include <array>
#include "DE2solver.h"
#include <iostream>>

namespace {
    constexpr double g = 9.807;   
}

Chain::Chain(double Length, int N, double A[2], double B[2], double M)
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

    Xt = GridFunction(L, std::vector<double>(N+1, 0.0), N, 1);
    Yt = GridFunction(L, std::vector<double>(N+1, 0.0), N, 1);

    CalculateCatenary(Length, A, B);
}

void Chain::CalculateCatenary(double Length, double A[2], double B[2])
{
    bool swapped = B[0] < A[0];
    double x1 = swapped ? B[0] : A[0], y1 = swapped ? B[1] : A[1];
    double x2 = swapped ? A[0] : B[0], y2 = swapped ? A[1] : B[1];

    double dx = x2 - x1, dy = y2 - y1;

    double r = std::sqrt(Length*Length - dy*dy) / dx;  
    double lo = 0, hi = 1;
    while (std::sinh(hi) / hi < r) hi *= 2;
    for (int it = 0; it < 200; it++)                   
    {
        double mid = 0.5 * (lo + hi);
        if (std::sinh(mid) / mid < r) lo = mid; else hi = mid;
    }
    double xi = 0.5 * (lo + hi);
    double a = dx / (2 * xi);

    double x0 = 0.5 * (x1 + x2) - a * std::atanh(dy / Length);
    double c  = y1 - a * std::cosh((x1 - x0) / a);

    double s1 = a * std::sinh((x1 - x0) / a);           

    std::vector<double> xs(N+1), ys(N+1);
    for (int i = 0; i <= N; i++)
    {
        double s = s1 + i * h;
        xs[i] = x0 + a * std::asinh(s / a);
        ys[i] = c + std::sqrt(a*a + s*s);
    }

    if (swapped)
    {
        std::reverse(xs.begin(), xs.end());
        std::reverse(ys.begin(), ys.end());
    }

    X = GridFunction(L, xs, N, 2);
    Y = GridFunction(L, ys, N, 2);
}

void Chain::Update(double dB[2], double dt)
{
    double vxN_old=Xt.F[N];
    double vyN_old=Yt.F[N];

    double vxN=dB[0]/dt;
    double vyN=dB[1]/dt;
 
    double axN = (vxN-vxN_old)/dt;
    double ayN = (vyN-vyN_old)/dt;

    if (axN != 0)
    {
        std::cout << axN << std::endl; 
    }

    double gamma=-12;
    double p = 200.;
    double q=100;
 
    std::array<std::vector<double>, 4> Cfs;
    for (int i = 0; i<4; i++){
        Cfs[i].resize(N+1);
    }

    for (int i= 0; i<N+1; i++)
    {
        Cfs[2][i]=pow(X.D1[i],2) + pow(Y.D1[i],2);
        Cfs[1][i]=0;
        Cfs[0][i]=-(pow(X.D2[i], 2) + pow(Y.D2[i], 2));
        Cfs[3][i]=(M/Length)*((pow(Xt.D1[i],2) + pow(Yt.D1[i],2))+(q*q/2)*(pow(X.D1[i], 2) + pow(Y.D1[i],2)-1) + p*(X.D1[i]*Xt.D1[i]+Y.D1[i]*Yt.D1[i])) + (gamma/2)*(X.D1[i]*Xt.D1[i]+Y.D1[i]*Yt.D1[i]);
    }

    double *BC0 = new double[3]{0,1,(M/Length)*g*Y.D1[0]};
    double *BCN = new double[3]{0, 1, (M/Length)*(g*Y.D1[N]+axN*X.D1[N]+ayN*Y.D1[N])-gamma*(X.D1[N]*Xt.F[N]+Y.D1[N]*Yt.F[N])};

    std::vector<double> T = solveDE2(N, h, BC0, BCN, Cfs);
    std::vector<double> Tl = GridFunction::Derivative(T, h, 1);
    std::vector<double> Tll = GridFunction::Derivative(T, h, 2);

    double s=0;
    // for (int i=0; i<N+1; i++)
    // {
    //     s+=Cfs[2][i]*Tll[i]+Cfs[1][i]*Tl[i]+Cfs[0][i]*T[i]+Cfs[3][i];
    // }
    // std::cout << s/(N+1) << std::endl;



    std::vector<double> Xt2(N+1);
    std::vector<double> Yt2(N+1);
    for (int i =1; i < N; i++)
    {
        Xt2[i]= (Length/M)*(T[i]*X.D2[i]+Tl[i]*X.D1[i] + gamma*Xt.F[i]);
        Yt2[i]=(Length/M)*(T[i]*Y.D2[i]+Tl[i]*Y.D1[i]+gamma*Yt.F[i])-g;
    }

    Xt2[0]=0;
    Yt2[0]=0;
    Xt2[N]=axN;
    Yt2[N]=ayN;

    std::vector<double> zeros(N+1, 0.0);

    Xt.Integrate(Xt2, zeros, dt);
    X.Integrate(Xt.F, zeros, dt);
    Yt.Integrate(Yt2, zeros, dt);
    Y.Integrate(Yt.F, zeros, dt);
}