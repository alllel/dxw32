#define STRICT
#include <windows.h>
#include <memory.h>
#include "pline.h"

void
Pline::_drop() {
  if (p_x != -1) {
    if (p_ymax == p_ymin) {
      OutPoint(p_x, p_ymin);
    } else {
      OutPoint(p_x, p_ymin);
      OutPoint(p_x, p_ymax);
    }
  }
  p_x = -1;
}

void
Pline::_ensure(int newlen) {
  pts.reserve(newlen);
}

void
Pline::OutPoint(int _x, int _y) {
  pts.emplace_back(_x,_y);
}

void
Pline::FilterPoint(int x, int y) {
  if (p_x != x) {
    _drop();
    p_x    = x;
    p_ymin = p_ymax = y;
  } else {
    if (y > p_ymax) p_ymax = y;
    if (y < p_ymin) p_ymin = y;
  }
}

void
Pline::Polyline(HDC hdc) {
  int i;
  int l;

  i = 0;
  do {
    l = Len() - i;
    if (l > MAXPOLY) l = MAXPOLY;
    ::Polyline(hdc, &pts[i], l);
    i += l - 1;
  } while (i < (Len() - 1));
}
