#include "fass.h"
#include "maths/algoritmos.h"
#include "tipomatlab/tipomatlab.h"
#include <QElapsedTimer>
#include <fcntl.h>
#include <sys/stat.h>
#include <gsl/gsl_blas.h>
#include <QFile>
#include <math.h>


// Variables locales
static Vector m_imgGridX;
static Vector m_imgGridY;
static Vector m_imgSampleX[RING_SAMPLERS];
static Vector m_imgSampleY[RING_SAMPLERS];
static Vector m_imgZi [RING_SAMPLERS];
static Vector m_fftParc;
static Vector m_fftAcum;
static Vector m_fint;
static Vector m_vmed;
static Matrix m_vcal;
static Vector m_wcal;
static Vector m_fit_result;
static int m_num_frames=0;

// Funciones locales
static void load_calib();
static void reduce_spectra(Vector& result, Vector& frecuencia, Vector& fint);


/********************************************************************************
 *
 *  Modulo profiler del FASS
 *
 * *****************************************************************************/
bool fass_profiler_init(int img_w, int img_h, int frames)
{
    // Cargar archivo de calibracion vcal y wcal
    load_calib();

    // Iniciar modulos
    algoritmos_init(img_w, img_h);
    m_num_frames = frames;

    // Grillas para los frames
    double xlim = (img_w-1.0)/2;
    double ylim = (img_h-1.0)/2;
    m_imgGridX = regspace(-xlim, 1, xlim);
    m_imgGridY = regspace(-ylim, 1, ylim);

    // Anillos sampleadores
    double r1 = (TEL_SZ_EXT/2)*0.67;
    double r2 = (TEL_SZ_EXT/2)*0.82;
    double dr = (r2-r1)/RING_SAMPLERS;

    // Pol2Cart
    for(int r=0; r<RING_SAMPLERS; r++)
    {
        m_imgSampleX[r] = zeros(RING_SZ);
        m_imgSampleY[r] = zeros(RING_SZ);

        double rad = (r1+dr*r)*OPTCONJ_DA/TEL_FOCAL/IMG_PIX_SZ+0.5;
        for(int ang=0; ang<RING_SZ; ang++)
        {
            double phi = ang*2*M_PI/RING_SZ;
            m_imgSampleX[r][ang] = rad*cos(phi);
            m_imgSampleY[r][ang] = rad*sin(phi);
        }
    }

    // Anillos sampleados e interpolados, aqui se almacena
    // su propia FFT tambien
    for(int i=0; i<RING_SAMPLERS; i++)
        m_imgZi[i] = zeros(RING_SZ);

    // FFT parcial por frame y FFT acumulado de los anillos
    m_fftParc = zeros(RING_SZ/2);
    m_fftAcum = zeros(RING_SZ/2);

    // Inicializar integracion de frecuencias
    Vector dfint  = zeros(RED_SPEC_SZ);
    m_fint = zeros(RED_SPEC_SZ);
    m_fint[0] = 1;
    dfint[0] = 0.84;
    for(int i=1; i<RED_SPEC_SZ; i++)
    {
        dfint[i]= dfint[i-1]+0.16;
        m_fint[i] = m_fint[i-1]+dfint[i];
    }

    // Iniciar vmed y resultado del fit
    m_fit_result = zeros(FIT_HSLOTS);
    m_vmed = zeros(RED_SPEC_SZ-2);

    // No hay calibracion, error
    return m_vcal.rows()==FIT_HSLOTS && m_vcal.cols()==FIT_NFREC &&
           m_wcal.lenght()==FIT_NFREC ;
}


/********************************************************************************
 *
 *  Agrega una imagen al calculador de profile
 *
 * *****************************************************************************/
void fass_profiler_push_image(const Matrix &image)
{    
    // No hay vcal, no hacer nada
    if(m_vcal.isEmpty()) return;

    // Calcular anillos interpolados a RING_SZ samples
    interp2_set(m_imgGridX, m_imgGridY, image);
    for(int i=0; i<RING_SAMPLERS; i++)
        interp2_eval(m_imgZi[i], m_imgSampleX[i], m_imgSampleY[i]);

    // Calcular FFT, resultado en m_imgZi
    for(int i=0; i<RING_SAMPLERS; i++)
        fft512_abs(m_imgZi[i]);

    // Promediar m_imgZi, guardar en m_fftParc
    mean_8x256(m_fftParc, m_imgZi);

    // Normalizar
    double dc = m_fftParc[0];
    m_fftParc /= dc;

    // Acumular
    m_fftAcum += m_fftParc;
}


/********************************************************************************
 *
 *  Calcula el perfil con las imagenes procesadas
 *
 * *****************************************************************************/
void fass_profiler_calcular(Vector& layers)
{
    // No hay vcal, no hacer nada
    if(m_vcal.isEmpty()) return;

    // Promedio de espectros y sacer el DC. Ahora son 255 samples
    m_fftAcum /= m_num_frames;
    for(int i=1; i<m_fftAcum.lenght(); i++)
        m_fftAcum[i-1] = m_fftAcum[i];
    m_fftAcum.truncate(m_fftAcum.lenght()-1);

    // Reducir espectro, 255->FIT_NFREC
    reduce_spectra(m_vmed, m_fftAcum, m_fint);

    // Fitear, resultado en fir
    lsqnonlin(m_fit_result, m_vmed, m_vcal, m_wcal);

    layers = m_fit_result;
}


/********************************************************************************
 *
 *  Reduce el espectro
 *
 * *****************************************************************************/
static void reduce_spectra(Vector& result, Vector& frecuencia, Vector& fint)
{
    int nint = fint.lenght()-2;
    result[0] = frecuencia[0];
    for(int i=1; i<nint; i++)
    {
        double frac1=fint[i-1] - fix(fint[i-1]);
        double frac2=fint[i]   - fix(fint[i]);

        int k1=fix(fint[i-1])+1;
        int k2=fix(fint[i]);
        double sum=0;
        for(int j=k1; j<=k2; j++)
            sum += frecuencia[j-1];
        result[i]= sum - frac1*frecuencia[fix(fint[i-1])-1]+frac2*frecuencia[fix(fint[i])-1];
    }
}


/********************************************************************************
 *
 *  MCarga calibracion desde archivo Vcal y Wcal
 *
 * *****************************************************************************/
static void load_calib()
{
    // Cargar archivo con calibracion
    struct stat finfo;
    if(stat(FASS_CALIB, &finfo) != 0 || finfo.st_size < (long)(5*sizeof (int)))
    {
        printf("load_calib(): archivo no existe\n");
        return;
    }

    QFile fcal(FASS_CALIB);
    fcal.open(QIODevice::ReadOnly);
    if(!fcal.isOpen())
    {
        printf("load_calib(): no se pudo abrir archivo\n");
        return;
    }

    int szs[3];
    ulong szhead = sizeof(int)*3;
    ulong db=fcal.read((char*)szs,szhead);
    if(db != szhead)
    {
        printf("load_calib(): no se pudo leer archivo o invalid_size\n");
        fcal.close();
        return;
    }
    printf("**%ld\n", db);

    long file_sz = szhead + sizeof(double)*szs[0]*szs[1] + sizeof(double)*szs[2];
    bool file_ok = rangeok(szs[0],1,1024) &&
                   rangeok(szs[1],1,1024) &&
                   rangeok(szs[2],1,1024) &&
                   finfo.st_size == file_sz;
    if(!file_ok)
    {
        printf("load_calib(): archivo incorrecto\n");
        fcal.close();
        return;
    }

    // leer
    m_vcal = zeros(szs[0],szs[1]);
    m_wcal = zeros(szs[2]);
    size_t sz_vcal = sizeof(double)*szs[0]*szs[1];
    size_t sz_wcal = sizeof(double)*szs[1];
    size_t rd1 = fcal.read((char*)m_vcal.data(),sz_vcal);
    size_t rd2 = fcal.read((char*)m_wcal.data(),sz_wcal);

    bool rdok = rd1==sz_vcal && rd2==sz_wcal;
    printf("%lld %lld * %lld %lld\n", rd1, rd2, sz_vcal, sz_wcal);

    if(!rdok)
    {
        printf("load_calib(): error al leer matrices\n");
        m_vcal.clear();
        m_wcal.clear();
    }
    else printf("load_calib(): lectura correcta vcal[%d %d] wcal[%d]\n", m_vcal.rows(), m_vcal.cols(), m_wcal.lenght());
    fcal.close();
}


/********************************************************************************
 *
 *  Devuelve resultados del ultimo calculo ejecutado
 *
 * *****************************************************************************/
void fass_profiler_results(Vector& coeficientes, Vector& vmed, Vector& vcalc)
{
    coeficientes = m_fit_result;
    vmed = m_vmed;

    // Mostrar resultado
    Vector q(FIT_NFREC);
    q.set(0);
    for(int i=0; i<FIT_HSLOTS; i++)
        for(int k=0; k<FIT_NFREC; k++)
            q[k] += m_fit_result[i]*m_vcal[i][k];

    vcalc = q;
}
