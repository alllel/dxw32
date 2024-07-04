#define MAXPOLY 512

class Pline {
  int len;
  int alloc;
  POINT* pts;
  int p_x, p_ymin, p_ymax;
  void _drop();
  void _ensure(int newlen);

 public:
  Pline();
  void OutPoint(int x, int y);
  void FilterPoint(int x, int y);
  void Finish();
  void Polyline(HDC);
  int Len();
  ~Pline();
};

inline Pline::Pline() {
  len   = 0;
  alloc = 0;
  pts   = nullptr;
  p_x   = -1;
}

inline Pline::~Pline() {
  if (pts) {
    delete pts;
  }
}

inline int
Pline::Len() {
  return len;
}

inline void
Pline::Finish() {
  _drop();
  _ensure(len);
}
