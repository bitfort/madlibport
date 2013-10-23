#ifndef MADLIB_MODULES_IMPALA_LINREG_INL_H
#include <cstdio>

#include "udf/udf.h"


using namespace impala;
using namespace impala_udf;
using namespace std;

/*! \brief Initializes the UDA state with zeros
 */
void LinRegInit(FunctionContext* context, StringVal* m);

/*! \brief Updates the input state with the given value
 */
void LinRegUpdate(FunctionContext* context, StringVal* input,
                  const StringVal& val, const DoubleVal &y);

/*! Combines two linear regression states
 */
void LinRegMerge(FunctionContext* context, const StringVal& src, StringVal* dst);

/*! \brief Computes the solution and returns the coefficient vector
 */
StringVal LinRegFinalize(FunctionContext* context, const StringVal& input);


/*! \brief Uses the trained model (output from LinRegFinalize) to predict an ex
 *
 * Does not allocate memory.  This is a simple dot product.
 * \param context Not used to allocate memroy
 * \param model the tarined model, result of LinRegFinalize
 * \param examp a double array of the example vector to predict
 * \return the predicted label of the example
 */
DoubleVal LinRegPredict(FunctionContext* context, const StringVal& model, 
                        const StringVal& examp);


#endif
