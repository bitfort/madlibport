
#ifndef IMPALA_PORT_MADLIB_LOGREG_H
#define IMPALA_PORT_MADLIB_LOGREG_H
#include "bismarck-inl.h"

namespace madlib_impala {

using namespace impala;
using namespace impala_udf;
using namespace std;

void LogrTrainInit(UdfContext* ctx, BismarckModel_t* st);
}
#endif
