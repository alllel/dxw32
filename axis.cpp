#define STRICT
#include <windows.h>
#include "dxw.h"
#include <cmath>
#include <cstdio>
#include <climits>
#include "axis.h"

void
Axis::trytick(int fact) {
  auto maxt = style & 0x00ff;
  if (maxt < 2) maxt = 2;
  int ord    = (int) ceil(log10(fabs(Len()) / fact / maxt));
  double val = fact * exp(log(10.0) * ord);
  if ((ceil(Max() / val) - floor(Min() / val)) > maxt) {
    val *= 10;
    ++ord;
  }
  if (val < tick.val) {
    tick.val  = val;
    tick.ord  = ord;
    tick.fact = fact;
  }
}

void
Axis::mkticks() {
  tick.val = fabs(Len());
  trytick(1);
  trytick(2);
  trytick(5);
  if ((style & AM_SNAP) == AS_SNAP) {
    double min = floor(Min() / tick.val + 0.01) * tick.val;
    double max = ceil(Max() / tick.val - 0.01) * tick.val;
    Range::Set(min, max);
  }
}

static int coord, dir, tl;
inline int
X(int l, int w) { return dir ? l : coord + w * tl; }
inline int
Y(int l, int w) { return dir ? coord + w * tl : l; }
//inline void Mto(HDC hdc,int l,int w){MoveToEx(hdc,X(l,w),Y(l,w),nullptr);}
//inline void Lto(HDC hdc,int l,int w){LineTo(hdc,X(l,w),Y(l,w));}
void
Line(HDC hdc, int l1, int w1, int l2, int w2) {
  POINT line[2];
  line[0].x = X(l1, w1);
  line[0].y = Y(l1, w1);
  line[1].x = X(l2, w2);
  line[1].y = Y(l2, w2);
  Polyline(hdc, line, 2);
}

int
Axis::Draw(HDC hdc, const RECT& rc, int tcklen) const {
  int start, end, height, shift, i, ta, nfactor, decplace;
  double w, factor;
  if (style & AS_NOAXIS) return 0;
  shift = tcklen;
  dir   = style & AM_DIRECT;
  if (dir == AS_VERT) {
    start  = rc.bottom;
    end    = rc.top;
    tcklen = -tcklen;
    if ((style & AM_SIDE) == AS_LOW) {
      ta     = TA_BASELINE | TA_RIGHT;
      coord  = rc.left;
      height = rc.right - rc.left;
    } else {
      ta     = TA_BASELINE | TA_LEFT;
      coord  = rc.right;
      height = rc.left - rc.right;
    }
  } else {
    start = rc.left;
    end   = rc.right;
    if ((style & AM_SIDE) == AS_LOW) {
      ta     = TA_CENTER | TA_TOP;
      coord  = rc.bottom;
      height = rc.top - rc.bottom;
    } else {
      ta     = TA_CENTER | TA_BOTTOM;
      coord  = rc.top;
      height = rc.bottom - rc.top;
    }
  }

  //line
  if (style & AM_MID) coord = tcklen;
  Line(hdc, start, 0, end, 0);
  if (style & AM_MID) return 0;

  //ticks

  if ((style & AM_SIDE) == AS_HIGH) tcklen = -tcklen;

  switch (style & AM_TICKS) {
    case AS_NOTICK:
      return 0;
    case AS_TICKOUT:
      tl    = tcklen;
      shift = 0;
      break;
    case AS_TICKIN:
      tl    = -tcklen;
      shift = 0;
      break;
    case AS_GRID:
      tl     = height;
      tcklen = (int)(-0.01 * height);
      break;
  }

  //mkticks();

  w = ceil(Min() / tick.val - 0.01) * tick.val;
  while (w <= Max() + tick.val * 0.01) {
    i = d2scr(w, start, end);
    Line(hdc, i, 0, i + shift, 1);
    w += tick.val;
  }

  //labels
  if ((style & AM_DIGITS) == AS_NODIGIT) return 0;

  tl = tcklen;
  w  = fabs(Max());
  if (w < fabs(Min())) w = fabs(Min());
  nfactor = floor((log10(fabs(w)) + 1) / 3);
  nfactor *= 3;
  factor   = exp(log(10.0) * nfactor);
  decplace = nfactor - tick.ord;
  if (decplace < 0) decplace = 0;
  w = floor(Min() / tick.val) * tick.val;
  while (w <= Max()) {
    i = d2scr(w, start, end);
    sprintf(buf, "%.*f", decplace, w / factor);
    Text(hdc, X(i, 2), Y(i, 2), buf, ta);
    w += tick.val;
  }
  return nfactor;
}

int
Range::d2scr(double x, int imin, int imax) const {
  double r = (x - Min()) / Len() * (imax - imin) + imin;
  if (r > INT_MAX) return INT_MAX;
  if (r < INT_MIN) return INT_MIN;
  return (int)r;
}
