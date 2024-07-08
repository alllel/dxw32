#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"

double
Gauge::P2T(size_t x) {
  for (auto const& P : Rates) {
    if (x < P.Np) return P.Tstart + x * P.rate;
    x -= P.Np;
  }
  auto const& P = Rates.back();
  return P.Tstart + P.Np * P.rate;
}

/******************************************************************************/

size_t
Gauge::T2P(double tim) {
  long np = 0;
  if (tim < Rates.front().Tstart) return 0;
  for (auto const& P : Rates) {
    auto local_shift = static_cast<size_t>((tim - P.Tstart) / P.rate);
    if (local_shift < P.Np) return local_shift + np;
    else
      np += P.Np;
  }
  return np - 1;
}

/***************************************************************************/
