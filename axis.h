#ifndef AXIS_H
#define AXIS_H

class Range {
  double _min, _len;

 public:
  void Set(double min, double max) {
    (*this) = Range { min, max };
  }
  void Set(Range const& r) {
    *this = r;
  }
  Range() { _min = _len = 0; }
  [[nodiscard]] double Min() const { return _min; }
  [[nodiscard]] double Max() const { return _min + _len; }
  [[nodiscard]] double Len() const { return _len; }

  [[maybe_unused]] Range(double min, double max) : _min { min }, _len { max - min } {}
  Range(Range const& r)            = default;
  Range& operator=(Range const& r) = default;
  [[nodiscard]] int d2scr(double x, int imin, int imax) const;
  [[nodiscard]] double scr2d(int i, int imin, int imax) const {
    return (i - imin) * Len() / (imax - imin) + Min();
  }
  void Incr(double d);
  void Incr(const Range& r);
};

struct fRECT {
  Range x, y;
};

inline void
Range::Incr(double d) {
  if (d > Max()) Set(Min(), d);
  else if (d < Min())
    Set(d, Max());
}

inline void
Range::Incr(const Range& r) {
  if (r.Max() > Max()) Set(Min(), r.Max());
  if (r.Min() < Min()) Set(r.Min(), Max());
}

inline int
x2scr(double x, fRECT& fr, RECT& rc) { return fr.x.d2scr(x, rc.left, rc.right); }

inline int
y2scr(double y, fRECT& fr, RECT& rc) { return fr.y.d2scr(y, rc.bottom, rc.top); }

inline double
scr2x(int ix, fRECT& fr, RECT& rc) { return fr.x.scr2d(ix, rc.left, rc.right); }

inline double
scr2y(int iy, fRECT& fr, RECT& rc) { return fr.y.scr2d(iy, rc.bottom, rc.top); }

#define AM_AXIS 0x8000
#define AS_NOAXIS 0x8000
#define AS_DRAXIS 0x0000

#define AM_TICKS 0x0300
#define AS_TICKOUT 0x0000
#define AS_TICKIN 0x0100
#define AS_GRID 0x0200
#define AS_NOTICK 0x0300

#define AM_DIGITS 0x0400
#define AS_NODIGIT 0x0400
#define AS_DIGIT 0x0000

#define AM_SNAP 0x0800
#define AS_NOSNAP 0x0800
#define AS_SNAP 0x0000

#define AM_DIRECT 0x1000
#define AS_HORIZ 0x1000
#define AS_VERT 0x0000

#define AM_SIDE 0x2000
#define AS_LOW 0x0000
#define AS_HIGH 0x2000

#define AM_MID 0x4000
#define AS_MID 0x4000
#define AS_NOMID 0x4000

struct Tick {
  int ord = 0, fact = 0;
  double val = 0;
  [[nodiscard]] double Ord() const { return val / fact; }
  Tick() = default;
};

struct Axis : public Range {
  UINT style = 15;
  Tick tick;
  void trytick(int);
  void mkticks();
  int Draw(HDC hdc, const RECT& rc, int tcklen);
  void Set(Range& r, UINT st) {
    Range::Set(r);
    style = st;
    mkticks();
  }
  void Set(Range const& r) {
    Range::Set(r);
    mkticks();
  }
  void Set(double min, double max, UINT st) {
    Range::Set(min, max);
    style = st;
    mkticks();
  }
  void Set(double min, double max) {
    Range::Set(min, max);
    mkticks();
  }
  Axis() = default;
};

class Gauge;

struct DrOpt {
  Axis T, P, I;
  int tickT, tickP, tickI;
  int DrCut;
  RECT rcG, rcCut;
  Range upd;
  void SetRect(RECT&);
  void SetCutRect(Gauge&);
  int TAxis(HDC hdc) { return T.Draw(hdc, rcG, tickT); }
  int PAxis(HDC hdc) { return P.Draw(hdc, rcG, tickP); }
  int IAxis(HDC hdc) { return I.Draw(hdc, rcG, tickT); }
  static char* Tscale(int);
  static char* Pscale(int, char*);
  static char* Iscale(int, char*);
  int T2scr(double t) { return T.d2scr(t, rcG.left, rcG.right); }
  int P2scr(double p) { return P.d2scr(p, rcG.bottom, rcG.top); }
  int I2scr(double i) { return I.d2scr(i, rcG.bottom, rcG.top); }
  double scr2T(int t) { return T.scr2d(t, rcG.left, rcG.right); }
  double scr2P(int p) { return P.scr2d(p, rcG.bottom, rcG.top); }
  double scr2I(int i) { return I.scr2d(i, rcG.bottom, rcG.top); }
};

#endif
