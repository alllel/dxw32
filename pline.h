#include <vector>
#define MAXPOLY 512

class Pline {
  std::vector<POINT> pts;
  int p_x=-1, p_ymin, p_ymax;
  void _drop();
  void _ensure(int newlen);

 public:
  Pline()=default;
  void OutPoint(int x, int y);
  void FilterPoint(int x, int y);
  void Finish();
  void Polyline(HDC);
  int Len();
  ~Pline()=default;
};


inline int
Pline::Len() {
  return pts.size();
}

inline void
Pline::Finish() {
  _drop();
  _ensure(Len());
}
