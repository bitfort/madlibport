#include <cstdio>

#include "test-macros.h"
#include "bismarck-common.h"

template <class T>
T* BismarckAllocate(void* ignore, size_t len) {
  return new T[len];
}

template <class T>
void BismarckFree(void* ignore, T* p) {
  delete [] p;
}


#include "mfact-inl.h"


using namespace hazy;

/*! Check that we're linked and can call a function :)
 */
int TEST_MFinit() {
  bismarck::bytea model;
  bismarck::BismarckMF<void*>::Init(NULL, &model);

  EXPECT_EQ(model.str, NULL);
  EXPECT_EQ(model.len, 0);
  return 1;
}

int TEST_MFstep() {
  bismarck::bytea model;
  bismarck::BismarckMF<void*>::Init(NULL, &model);

  // We will factor
  // L * R = [ [0, 4], [0, 4] ]
  //
  // We expect to factor it into (since mean = 2)
  // L = [sqrt(2) sqrt(2)]
  // R = [-sqrt(2) sqrt(2)]

  double mean = (0 + 4 -0 + 4) / 4.0;

  uint64_t rowd[2] = {2, 2};
  uint64_t cold[2] = {2, 2};
  bismarck::bytea rowdeg = {(char*) rowd, sizeof(uint64_t)*2};
  bismarck::bytea coldeg = {(char*) cold, sizeof(uint64_t)*2};

  double step = 0.1;

  for (size_t i = 0; i < 100; i++) {
    bismarck::BismarckMF<void*>::Step(NULL, 0, 0, 0, &model, step, 0, mean, 1, rowdeg, coldeg);
    bismarck::BismarckMF<void*>::Step(NULL, 0, 1, 4, &model, step, 0, mean, 1, rowdeg, coldeg);
    bismarck::BismarckMF<void*>::Step(NULL, 1, 0, 0, &model, step, 0, mean, 1, rowdeg, coldeg);
    bismarck::BismarckMF<void*>::Step(NULL, 1, 1, 4, &model, step, 0, mean, 1, rowdeg, coldeg);
  }

  EXPECT_NEAR(bismarck::MFPredict<void*>(NULL, model, 0, 0), 0.0, 1e-5);
  EXPECT_NEAR(bismarck::MFPredict<void*>(NULL, model, 0, 1), 4.0, 1e-5);
  EXPECT_NEAR(bismarck::MFPredict<void*>(NULL, model, 1, 0), 0.0, 1e-5);
  EXPECT_NEAR(bismarck::MFPredict<void*>(NULL, model, 1, 1), 4.0, 1e-5);


  bismarck::MFStrL<void*>(NULL, model);
  bismarck::MFStrR<void*>(NULL, model);



  return 1;
}


int main() {
  RUNTEST(TEST_MFinit);
  RUNTEST(TEST_MFstep);
}
