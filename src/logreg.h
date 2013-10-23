
#ifndef IMPALA_PORT_MADLIB_LOGREG_H
#define IMPALA_PORT_MADLIB_LOGREG_H

namespace hazy {
namespace bismarck {


/*! \brief Implements Logistic Regression using BIsmarck
 *
 * This implements the basic functions required by bismarck
 * \tparam Context the bismarck context type, used for memory allocation,
 * there should exist void* BismarckAllocate(Context *c, size_t bytes).
 */
template <class Context>
class BismarckLogr {
 public:
  /*! \brief Initializes an empty Logistic model
   *
   * The memory for the model will be allocated on the first call to Step
   */
  static void Init(Context* ctx, bytea *m);

  /*! \brief Computes the objective value of the given example
   *
   * \param v the example vector (double array)
   * \param y the label for the example
   * \param model an Logistic model (double array)
   */
  static double Loss(const bytea &v, 
                     bool y,
                     const bytea &model);

  /*! \brief Takes an IGD step using the given exmaple
   * 
   * \param ctx the context to allocate memory with
   * \param val the example 
   * \param y the label for the example
   * \param input the current model
   */
  static void Step(Context* ctx, 
                   const bytea& val, const bool &y,
                   bytea *input, double step, double mu);

  /*! \brief Combines 2 Logistic models together
   */
  static void Merge(Context* ctx, const bytea &src, 
                     const bytea &dst);

  /*! \brief Combines two losses
   * Simply adds them together
   */
  static double LossMerge(double a, double b) { return a + b; }
};

/*! \brief Predicts the label for the example using the given model
 */
template <class Context>
double LogrPredict(Context* ctx, const bytea &ex, const bytea &model);



}
}
#endif
