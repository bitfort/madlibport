#include <cstdio>

#include "udf/uda-test-harness.h"
#include "madport/port-dbconnector-inl.h"
#include "metaport/modules/linreg-inl.h"

using namespace impala;
using namespace impala_udf;
using namespace std;

// XXX dump all of madlib inout our namespace, this is a little ugly
using namespace madlib;

typedef dbconnector::mainmem::MutableByteString LRModel;
typedef MutableNativeColumnVector LRVector;

/*! \brief Initializes the UDA state with zeros
 */
void LinRegInit(UdfContext* context, StringVal* m) {
  PortAllocator pa(context);
  // get a handle to our allocated state
  madlib::MemHandle<char> state = madlib::modules::regress::LinrInit(pa);
  m->len = state.size;
  m->ptr = reinterpret_cast<uint8_t*>(state.ptr);
}

/*! \brief Updates the input state with the given value
 */
void LinRegUpdate(UdfContext* context, const StringVal& val, StringVal* input) {
  PortAllocator pa(context);

  // convert to types that MADlib expects
  madlib::MemHandle<char> state = {input->len, (char*)input->ptr};
  size_t len_val = val.len/sizeof(double);
  double *v = (double*) val.ptr;

  // the label is the first entry in the array
  madlib::MemHandle<char> new_state = 
      madlib::modules::regress::LinrTransition(pa, state, &v[1], 
                                                     len_val-1, v[0]);

  // clean up memory if the transition function re-allocated
  if (input->ptr != (uint8_t*) new_state.ptr) {
    pa.Free(input->ptr);
  }

  // state and new_state may or maynot be backed by the same memory
  // In particular, if this is the first call to transition, these will
  // be backed by different memory
  input->ptr = (uint8_t*) new_state.ptr;
  input->len = new_state.size;
}

void LinRegMerge(UdfContext* context, const StringVal& src, StringVal* dst) {
  PortAllocator pa(context);
  madlib::MemHandle<char> statea = {dst->len, (char*)dst->ptr};
  madlib::MemHandle<char> stateb = {src.len, (char*)src.ptr};

  madlib::MemHandle<char> combin = 
      madlib::modules::regress::LinrMerge(pa, statea, stateb);
  dst->ptr = (uint8_t*) combin.ptr;
  dst->len = combin.size;
}

/*! \brief Computes the solution and returns the coefficient vector
 */
StringVal LinRegFinalize(UdfContext* context, const StringVal& input) {
  PortAllocator pa(context);

  // convert to types that MADlib expects
  madlib::MemHandle<char> state = {input.len, (char*)input.ptr};

  madlib::MemHandle<double> coef = 
      madlib::modules::regress::LinrFinal(pa, state);

  printf("ans = %f %f\n", coef.ptr[0], coef.ptr[1]);
  StringVal sv((uint8_t*) coef.ptr, coef.size*sizeof(double));
  return sv;
}

/* Exeutes a simple test using a 2x2 example
 */
int TEST_linreg() {
  UdaTestHarness<StringVal, StringVal, StringVal> test1(
      LinRegInit, LinRegUpdate, LinRegMerge, 
      NULL, LinRegFinalize);
  vector<StringVal> no_nulls;

  // First entry is the label
  double ex1[3] = {5.0, 1.0, 2.0};
  double ex2[3] = {8.0, 2.0, 3.0};

  // Create the example input
  no_nulls.resize(2);
  no_nulls[0].ptr = (uint8_t*) ex1;
  no_nulls[0].len = 3 * sizeof(double);
  no_nulls[1].ptr = (uint8_t*) ex2;
  no_nulls[1].len = 3 * sizeof(double);

  // create the expected anser
  StringVal ans;
  double coef[2] = {1.0, 2.0};
  ans.ptr = (uint8_t*) coef;
  ans.len = 2 * sizeof(double);

  //test it
  bool b = test1.Execute(no_nulls, ans);
  return (int)b;
}

int main(int argc, char** argv) {
  //impala::InitGoogleLoggingSafe(argv[0]);
//  ::testing::InitGoogleTest(&argc, argv);
  //return RUN_ALL_TESTS();
  RUNTEST(TEST_linreg);
}
