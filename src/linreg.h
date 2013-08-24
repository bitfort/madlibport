#ifndef MADLIB_MODULES_IMPALA_LINREG_INL_H
#include <cstdio>

#include "udf/udf.h"

namespace madlib_impala {

using namespace impala;
using namespace impala_udf;
using namespace std;

/*! \brief Initializes the UDA state with zeros
 */
void LinRegInit(UdfContext* context, StringVal* m);

/*! \brief Updates the input state with the given value
 */
void LinRegUpdate(UdfContext* context, const StringVal& val, const DoubleVal &y, 
                  StringVal* input);

void LinRegMerge(UdfContext* context, const StringVal& src, StringVal* dst);

/*! \brief Computes the solution and returns the coefficient vector
 */
StringVal LinRegFinalize(UdfContext* context, const StringVal& input);

} // namespace impala
#endif
