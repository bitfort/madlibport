#include <limits.h>


// Check a simple construction
int AnyType_ctor() {
  using namespace madlib::dbconnector::mainmem;
  AnyType anyt(7);
  return anyt.getAs<int>() == 7;
}


int AnyType_tuple() {
  using namespace madlib::dbconnector::mainmem;
  AnyType anyt;
  double d = 1.2;
  anyt << 7 << d;

  return (anyt[0].getAs<int>() == 7) && 
    (anyt[1].getAs<double>() == 1.2);
}


// Check a simple construction
int AnyType_assign() {
  using namespace madlib::dbconnector::mainmem;
  AnyType anyt(7);
  AnyType t2;
  t2 = anyt;
  return t2.getAs<int>() == 7;
}



/*! Run all of the tests for AnyType
 */
void TEST_AnyType() {
  RUNTEST(AnyType_ctor)
  RUNTEST(AnyType_tuple)
  RUNTEST(AnyType_assign)
}
