#ifndef GAUGE_H
#define GAUGE_H

/******************************************************************************/
#include "axis.h"
#include "twnd.h"
#include "cutwnd.h"
#include "pline.h"
#include "Experiment.h"
#include <dos.h>
#include <ranges>
#include <utility>


/******************************************************************************/

class RT;
struct Time {
  short hour=0;
  short min=0;
  short sec=0;
};
struct Date {
  short year=0;
  char month=0;
  char day=0;
};


class Gauge : public CutWnd {

 public:
  // IXC file date
  // Head
  double radius = 0.0;
  int angle     = 0;
  int number    = 0;
  char g_type   = '\0';

  char ChNum[6] = "";
  char ID[17]   = "";
  double dV     = 0.0;
  double V0     = 0.0;
  char unit[4]  = "";
  Time time;
  Date date;
  struct Piece {
    size_t Np;
    double rate;
    double Tstart;
  };
  std::vector<Piece> Rates;
  std::shared_ptr<Experiment> Exp;
  // Modified parms
  double m_Lower = 0.0, m_Upper = 0.0;
  double Zero_corr = 0.0;
  size_t start = 0, final = 0;
  // Calculated
  size_t FilePos = 0;
  size_t count   = 0;
  ssize_t pts[2] = { -1, -1 };
  double Ipos    = 0.0;
  double Ineg    = 0.0;
  double Itot    = 0.0;
  std::vector<short int> val;
  std::vector<float> imp;
  double Imax = 0.0, Imin = 0.0;

  std::unique_ptr<Pline> PolyP, PolyI;
  ssize_t Curr = -1;
  bool rcValid = false, frcValid = false;
  fRECT fr;
  RECT rcGrf;
  bool drawI = false;

  // Methods
  // Constructors & destructors
  static int nGauges;
  explicit Gauge(std::shared_ptr<Experiment> E);
  void Create();
  ~Gauge() override;

  // Memory Management
  void LockD();
  void FreeD();
  bool ReadD();

  void LockI();
  void FreeI();
  void CalcI();


  // Data preparation
  void Setup();
  void ULSetup();

  // Pointer manipulation
  void DrawPointer(HDC);
  void SetPointer(WORD);
  void MovePointer(int);
  void HomePointer(bool);
  void PointerSet(long);
  void PointSet(int);
  void SetDigitize();

  // Cutting
  int GetCutRC(RECT& rc) override;
  void FinishCut(RECT& rc) override;
  void SetSize(HWND, RECT&) override;
  void SetInfo();

  // Menu commands & window messages
  int Write(std::fstream& hDAT, std::fstream& hIXC);
  void AfterWrite(std::shared_ptr<Experiment> E, size_t& Pos);
  void AcceptZero();
  void AcceptZero2();
  void WriteASCII();
  void Restore();

  void Redraw();
  void Draw(HDC, RECT&, DCtype, RECT*) override;
  bool Command(WPARAM cmd) override;
  bool WinProc(Msg& M) override;
  void Plot(HDC, DrOpt&);

  // Auxilary
  void Link(std::shared_ptr<Experiment> E) {
    Exp = std::move(E);
  }
  auto FileName() { return Exp->DAT(); }
  char* WinTitle();
  void SetTitle() { SetWindowText(hWnd, WinTitle()); }
  char* PicName() override;

  // Data access
  [[maybe_unused]] double I2D(short int v) const { return V0 + v * dV - Zero_corr; }
  short int D2I(double v) const { return (short int) ((v - V0 + Zero_corr) / dV); }

  double P2T(size_t x);
  size_t T2P(double);

  double Val(size_t i);

  float& Imp(size_t i);

  double Pp(int i) { return Val(pts[i]); }
  double Tp(int i) { return P2T(pts[i]); }
  double Start() { return P2T(start); }
  double Final() { return P2T(final); }
  double Upper() {
    if (m_Upper == m_Lower) ULSetup();
    return m_Upper;
  }
  double Lower() {
    if (m_Upper == m_Lower) ULSetup();
    return m_Lower;
  }
};

std::shared_ptr<Gauge> GaugeByWnd(HWND);
std::shared_ptr<Gauge> GaugeByChNum(char*);

inline auto
GaugeIterator() { return WindowIterator<Gauge>();}

[[maybe_unused]]
inline std::shared_ptr<Gauge>
FirstGauge() {
  for (auto G : GaugeIterator()) {
    return G;
  }
  return {};
}

inline std::shared_ptr<Experiment>
FirstExp() {
  for (auto G : GaugeIterator()) {
    if (G->Exp) return G->Exp;
  }
  return {};
}

#include "gaug_inl.h"

#endif //GAUGE_H
