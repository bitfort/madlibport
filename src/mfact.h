#ifndef HAZY_BISMARCK_MFACT_H
#define HAZY_BISMARCK_MFACT_H


namespace hazy {
namespace bismarck {


/*! \brief Implements matrix factorization
 *
 * This implements the basic functions required by bismarck
 * \tparam Context the bismarck context type, used for memory allocation,
 * there should exist void* BismarckAllocate(Context *c, size_t bytes).
 */
template <class Context>
class BismarckMF {
 public:
  /*! \brief Initializes an empty model
   *
   * The memory for the model will be allocated on the first call to Step
   */
  static void Init(Context* ctx, bytea *m);

  /*! \brief Computes the squared difference between predicted and actual
   *
   * \param row the row to predict
   * \param col the column to predict
   * \param val the actual value to test against
   * \param model an trained model
   */
  static double Loss(size_t row, size_t col, double val,
                     const bytea &model);

  /*! \brief Takes an IGD step using the given exmaple
   * 
   * \param ctx the context to allocate memory with
   * \param val the example 
   * \param y the label for the example
   * \param input the current model
   */
  static void Step(Context* ctx, size_t row, size_t col, double val,
                   bytea *mod, double step, double mu, double mean, 
                   size_t rank, const bytea &rowdeg, const bytea &coldeg);

  /*! \brief Combines 2 models together
   */
  static void Merge(Context* ctx, const bytea &src, 
                     bytea *dst);

  /*! \brief Combines two losses
   * Simply adds them together
   */
  static double LossMerge(double a, double b) { return a + b; }
};

/*! \brief Predicts the label for the example using the given model
 */
template <class Context>
bool MFPredict(Context* ctx, size_t row, size_t col, double mean, 
               const bytea &model);

}
}
#endif
