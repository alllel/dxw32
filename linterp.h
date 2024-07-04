#ifndef _LINTERP_H
#define _LINTERP_H

#include <stddef.h>

class linterp {
 protected:
  unsigned int length;
  float* X;
  float* F;

 public:
  unsigned autodelete : 1;
  void recalculate(void);
  linterp(void) {
    length     = 0;
    X          = nullptr;
    F          = nullptr;
    autodelete = 0;
  }
  void reset(unsigned int l, float* x, float* f) {
    length = l;
    X      = x;
    F      = f;
    recalculate();
  }
  linterp(unsigned int l, float* x, float* f) {
    reset(l, x, f);
    autodelete = 0;
  }
  float operator()(float);
  float& operator[](int i) { return F[i]; }
  unsigned int len() { return length; }
  float range() { return X[length - 1] - X[0]; }
  float xmin() { return X[0]; }
  float xmax() { return X[length - 1]; }
  float ymin();
  float ymax();
  ~linterp() {
    if (autodelete) {
      delete X;
      delete F;
    }
  }
};
#endif
