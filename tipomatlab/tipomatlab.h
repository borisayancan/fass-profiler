#ifndef TIPOMATLAB_H
#define TIPOMATLAB_H
#include "maths/math_tipos.h"
#include "plot.h"
#include "imshow.h"

// Pseudofuncione tipo matlab
#define imshow(A)           { Imshow im(A); im.exec(); }
#define plot(A)             { Plot p(A.data(), A.lenght()); p.exec(); }
#define plot_hold_on(A,B)   { Plot p(A.data(), A.lenght()); p.add(B.data(), B.lenght()); p.exec(); }
#define bar(A)              { Plot p(A.data(), A.lenght(), PLOT_BAR); p.exec(); }
inline Matrix zeros(int r, int c)   { Matrix ret(r,c); ret.set(0); return  ret; }
inline Matrix ones(int r, int c)    { Matrix ret(r,c); ret.set(1); return ret; }
inline Vector zeros(int len)        { Vector ret(len); ret.set(0); return  ret; }
inline Vector regspace(double start, double step, double end)
{
    int nstep = (end-start)/step+1+1e-6;
    Vector ret(nstep);
    for(int x=0; x<nstep; x++)
    {
        double val = start + step*x;
        ret[x] = val;
    }
    return ret;
}

inline Matrix meshgrid(double start, double step, double end)
{
    int nstep = (end-start)/step+1+1e-6;
    Matrix ret(nstep,nstep);
    for(int x=0; x<nstep; x++)
    {
        double val = start + step*x;
        for(int k=0; k<nstep; k++)
            ret[k][x] = val;
    }
    return ret;
}

inline void display(const Vector& d, const char* titulo=nullptr, bool completo=false)
{
    if(titulo!=NULL) printf("%s len=%d\n", titulo, d.lenght());
    else printf("len=%d\n", d.lenght());

    const double* ptr = d.data();
    for(int i=0; i<d.lenght(); i++)
    {
        printf("%.4f ", ptr[i]);
        if(!completo && i>=15)
        {
            printf("...");
            break;
        }
    }

    printf("\n");
}


inline void display(const Matrix& d, const char* titulo=nullptr, bool completo=false)
{
    if(titulo!=NULL) printf("%s [%d %d]\n", titulo, d.rows(), d.cols());
    else printf("[%d %d]\n", d.rows(), d.cols());

    const double* ptr = d.data();
    for(int i=0; i<d.rows(); i++)
    {
        for(int k=0; k<d.cols(); k++)
        {
            printf("%f ", ptr[i*d.cols() +k]);
            if(!completo && k>=15)
            {
                printf("...");
                break;
            }
        }

        if(!completo && i>=15)
        {
            printf("...");
            break;
        }
    }

    printf("\n");
}



#endif // TIPOMATLAB_H
