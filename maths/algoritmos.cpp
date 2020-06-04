#include "algoritmos.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_interp2d.h>
#include <gsl/gsl_spline2d.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_multifit_nlinear.h>


// GSL FFT de 512 pts
gsl_fft_real_wavetable * m_gslfft_real;
gsl_fft_real_workspace * m_gslfft_work;
gsl_spline2d     *m_gsl_spline   ;
gsl_interp_accel *m_gsl_xacc ;
gsl_interp_accel *m_gsl_yacc ;



/*******************************************************************************
 *
 *  Inicio del modulo de algoritmos
 *
 * ****************************************************************************/
void algoritmos_init(int im_w, int im_h)
{
    m_gslfft_work = gsl_fft_real_workspace_alloc (512);
    m_gslfft_real = gsl_fft_real_wavetable_alloc (512);

    m_gsl_spline   = gsl_spline2d_alloc(gsl_interp2d_bilinear, im_w, im_h);
    m_gsl_xacc = gsl_interp_accel_alloc();
    m_gsl_yacc = gsl_interp_accel_alloc();
}


/*******************************************************************************
 *
 *  interp2, establece la imagen a interpolar
 *
 * ****************************************************************************/
void interp2_set(Vector &ximg, Vector& yimg, const Matrix &src)
{
    gsl_spline2d_init(m_gsl_spline, ximg.data(), yimg.data(), src.data(),
                      src.cols(), src.rows());
}


/*******************************************************************************
 *
 *  interp2, consulta puntos interpolados
 *
 * ****************************************************************************/
void interp2_eval(Vector &res, Vector &sample_x, Vector &sample_y)
{
    for(int i=0; i<sample_x.lenght(); i++)
        res[i] = gsl_spline2d_eval(m_gsl_spline, sample_x[i], sample_y[i], m_gsl_xacc, m_gsl_yacc);
}


/*******************************************************************************
 *
 *  Calcula la transformada de fourier de 512 puntos del vector
 *  indicado
 *
 * ****************************************************************************/
void fft512_abs(Vector& res)
{
    double* d = res.data();
    gsl_fft_real_transform (res.data(), 1, 512,
                            m_gslfft_real, m_gslfft_work);

    d[0] = d[0]*d[0];
    for(int i=1; i<256; i++)
    {
        double real = d[2*i-1];
        double imag = d[2*i];
        d[i] = real*real + imag*imag;
    }
}


/*******************************************************************************
 *
 *  Calcula el promedio del array de 8 vectores indicados en src
 *
 * ****************************************************************************/
void mean_8x256(Vector &result, Vector src[8])
{
    int elems=result.lenght();
    for(int i=0; i<elems; i++)
    {
        double suma=0;
        for(int k=0; k<8; k++)
            suma += src[k][i];
        result[i]=suma/8.;
    }
}


/*******************************************************************************
 *
 *  Estructura con datos para el solver de lsqnonlin
 *
 * ****************************************************************************/
struct T_LsqSolver
{
    Matrix* vcal;
    Vector* vmed;
    Vector* wcal;
    Vector* reconstructed;
};


/*******************************************************************************
 *
 *  Callback de lsqnonlin, calcula error con los coeficientes
 *  x de la iteracion actual
 *
 * ****************************************************************************/
static int lsq_callback (const gsl_vector * x, void *data, gsl_vector * f)
{
    T_LsqSolver* d = (T_LsqSolver*)data;
    int n=d->vcal->cols();
    int q=d->vcal->rows();

    // Calcular resultado usando los coeficientes actuales xi
    d->reconstructed->set(0);
    double* reconstr = d->reconstructed->data();
    for(int i=0; i<q; i++)
    {
        double* vcal = d->vcal->data();
        double xi = gsl_vector_get(x,i);
        if(xi<0)
        {
            for(int k=0; k<n; k++)
            {
                double costo = xi * vcal[i*n +k];
                reconstr[k] += costo*xi*50;
            }
        }
        else
        {
            for(int k=0; k<n; k++)
                reconstr[k] += xi * vcal[i*n +k];
        }
    }

    // Calcular errores con los coeficientes actuales
    double* vmed = d->vmed->data();
    double* wcal = d->wcal->data();
    for(int i=0; i<n; i++)
    {
        double error = vmed[i]-reconstr[i];
        error *= wcal[i]*1000;
        gsl_vector_set(f, i,  error);
    }

    return GSL_SUCCESS;
}


/*******************************************************************************
 *
 *  lsqnonlin, algoritmo trust-region. Se agrega low boundary en callback
 *
 * ****************************************************************************/
int lsqnonlin(Vector& result, Vector &vmed, Matrix &vcal, Vector &wcal)
{
    // GSL
    gsl_multifit_nlinear_workspace *w;
    gsl_multifit_nlinear_fdf fdf;
    const gsl_multifit_nlinear_type *T = gsl_multifit_nlinear_trust;
    gsl_multifit_nlinear_parameters fdf_params = gsl_multifit_nlinear_default_parameters();

    Vector reconstructed(vcal.cols());
    T_LsqSolver solver;
    solver.vcal = &vcal;
    solver.vmed = &vmed;
    solver.wcal = &wcal;
    solver.reconstructed = &reconstructed;

    /* define the function to be minimized */
    fdf.f = lsq_callback;
    fdf.df = NULL;          /* set to NULL for finite-difference Jacobian */
    fdf.fvv = NULL;         /* not using geodesic acceleration */
    fdf.n = vcal.cols();    // numero de funciones
    fdf.p = vcal.rows();    // numero de variabes
    fdf.params = &solver;

    /* allocate workspace with default parameters */
    w = gsl_multifit_nlinear_alloc (T, &fdf_params, vcal.cols(), vcal.rows());

    /* initialize solver with starting point and weights */
    Vector sol(vcal.rows());
    sol.set(1);

    gsl_vector_view x = gsl_vector_view_array (sol.data(), sol.lenght());
    gsl_multifit_nlinear_winit (&x.vector, NULL, &fdf, w);

    int info;
    int res = gsl_multifit_nlinear_driver(5000, 1e-9, 1e-9, 1e-9,
                                         NULL, NULL, &info, w);

    for(int i=0; i<vcal.rows(); i++)
        result[i] = gsl_vector_get(w->x, i);

    return res;
}
