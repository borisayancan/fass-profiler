#ifndef ALGORITMOS_H
#define ALGORITMOS_H
#include <math.h>
#include "math_tipos.h"


/*******************************************************************************
 *
 *  Algoritmos matematicos varios
 *
 * ****************************************************************************/
void algoritmos_init(int im_w, int im_h);
void interp2_set(Vector &ximg, Vector& yimg, const Matrix &src);
void interp2_eval(Vector &res, Vector &sample_x, Vector &sample_y);
void fft512_abs(Vector& res);
void mean_8x256(Vector &result, Vector src[8]);
int lsqnonlin(Vector &result, Vector& vmed, Matrix& vcal, Vector& wcal);



#endif // ALGORITMOS_H
