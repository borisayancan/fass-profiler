#ifndef FASS_PROFILER_H
#define FASS_PROFILER_H
#include "tipos.h"

bool fass_profiler_init(int img_w, int img_h, int frames);
void fass_profiler_push_image(const Matrix& matrix);
void fass_profiler_calcular(Vector &layers);
void fass_profiler_results(Vector& coeficientes, Vector& vmed, Vector& vcalc);


#endif // FASS_PROFILER_H
