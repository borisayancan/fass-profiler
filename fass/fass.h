#ifndef FASS_CONFIG_H
#define FASS_CONFIG_H
#include "tipos.h"
#include "fass_profiler.h"

// Numero de anillos para samplear pupila
#define RING_SAMPLERS   8

// Numero de samples por anillo
#define RING_SZ         512

// Telescopio
#define TEL_SZ_INT      0.14
#define TEL_SZ_EXT      0.3
#define TEL_FOCAL       2.4

// Imagen de camara
#define IMG_PIX_SZ      (2.9e-6*5)  //  Tamaño pixel binneado

// Conjugacion
#define OPTCONJ_DIST    -400
#define OPTCONJ_DA      0.01431412

// Reduccion de espectro
#define RED_SPEC_SZ     20

// Fiteo
#define FIT_NFREC       18
#define FIT_HSLOTS      15

// Archivo con Vcal,Wcal
#define FASS_CALIB      "../calprofiler.bin"


// Archivo con frames
#pragma pack(push)
#pragma pack(1)
struct T_FassFile
{
    int width;
    int height;
    char comentario[1024];
    u16 frame_start;
};
#pragma pack(pop)

#endif // FASS_CONFIG_H
