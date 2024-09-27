#ifndef AXIS_H
#define AXIS_H

class Range {
  double m_min, m_len;

 public:
  void Set(double min, double max) {
    (*this) = Range { min, max };
  }
  void Adjust(double v) {
    Set(std::min(Min(), v), std::max(Max(), v));
  }
  Range() { m_min = m_len = 0; }
  [[nodiscard]] double Min() const { return m_min; }
  [[nodiscard]] double Max() const { return m_min + m_len; }
  [[nodiscard]] double Len() const { return m_len; }

  [[maybe_unused]] Range(double min, double max) : m_min { min }, m_len { max - min } {}
  Range(Range const& r)            = default;
  Range& operator=(Range const& r) = default;
  [[nodiscard]] int d2scr(double x, int imin, int imax) const;
  [[nodiscard]] double scr2d(int i, int imin, int imax) const {
    return (i - imin) * Len() / (imax - imin) + Min();
  }
};

struct fRECT {
  Range x, y;
};

inline int
x2scr(double x, fRECT& fr, RECT& rc) { return fr.x.d2scr(x, rc.left, rc.right); }

inline int
y2scr(double y, fRECT& fr, RECT& rc) { return fr.y.d2scr(y, rc.bottom, rc.top); }

inline double
scr2x(int ix, fRECT& fr, RECT& rc) { return fr.x.scr2d(ix, rc.left, rc.right); }

inline double
scr2y(int iy, fRECT& fr, RECT& rc) { return fr.y.scr2d(iy, rc.bottom, rc.top); }

constexpr unsigned short AM_AXIS=0x8000;
constexpr unsigned short AS_NOAXIS=0x8000;
constexpr unsigned short AS_DRAXIS=0x0000;

constexpr unsigned short AM_TICKS=0x0300;
constexpr unsigned short AS_TICKOUT=0x0000;
constexpr unsigned short AS_TICKIN=0x0100;
constexpr unsigned short AS_GRID=0x0200;
constexpr unsigned short AS_NOTICK=0x0300;

constexpr unsigned short AM_DIGITS=0x0400;
constexpr unsigned short AS_NODIGIT=0x0400;
constexpr unsigned short AS_DIGIT=0x0000;

constexpr unsigned short AM_SNAP=0x0800;
constexpr unsigned short AS_NOSNAP=0x0800;
constexpr unsigned short AS_SNAP=0x0000;

constexpr unsigned short AM_DIRECT=0x1000;
constexpr unsigned short AS_HORIZ=0x1000;
constexpr unsigned short AS_VERT=0x0000;

constexpr unsigned short AM_SIDE=0x2000;
constexpr unsigned short AS_LOW=0x0000;
constexpr unsigned short AS_HIGH=0x2000;

constexpr unsigned short AM_MID=0x4000;
constexpr unsigned short AS_MID=0x4000;
constexpr unsigned short AS_NOMID=0x4000;

struct Tick {
  int ord = 0, fact = 0;
  double val = 0;
  Tick() = default;
};

struct Axis : public Range {
  UINT style = 15;
  Tick tick;
  void trytick(int);
  void mkticks();
  int Draw(HDC hdc, const RECT& rc, int tcklen) const;
  void Set(double min, double max, UINT st) {
    Range::Set(min, max);
    style = st;
    mkticks();
  }
  void SetRng(double min, double max) {
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
  int TAxis(HDC hdc) const { return T.Draw(hdc, rcG, tickT); }
  int PAxis(HDC hdc) const { return P.Draw(hdc, rcG, tickP); }
  int IAxis(HDC hdc) const { return I.Draw(hdc, rcG, tickT); }
  static char* Tscale(int);
  static char* Pscale(int, char*);
  static char* Iscale(int, char*);
  [[nodiscard]] int T2scr(double t) const { return T.d2scr(t, rcG.left, rcG.right); }
  [[nodiscard]] int P2scr(double p) const { return P.d2scr(p, rcG.bottom, rcG.top); }
  [[nodiscard]] int I2scr(double i) const { return I.d2scr(i, rcG.bottom, rcG.top); }
};

#endif
