#ifndef _SPECTRUM_H_
#define _SPECTRUM_H_
#include "axis.h"

class spectrum : public CutWnd {
  Gauge*G=nullptr;
  //	HLOCAL Abs,Phase;
  long N;
  std::vector<double> abs;
  double AbsMax;
  std::vector<double> phase;
  double FrScale;
  long st, fi;
  double Amin, Amax;
  Axis Fr, Am;
  RECT rcG;
  void Draw(HDC hdc, RECT& rc, DCtype t, RECT* rcUpd = nullptr) override;
  bool Command(WPARAM cmd) override;
  bool WinProc(Msg& M) override;
  char* PicName() override;
  //CutWnd
  int GetCutRC(RECT&) override;
  void SetSize(HWND, RECT&) override;
  void FinishCut(RECT&)override;

 public:
  spectrum(Gauge* _g);
  void LockData();
  void UnlockData();
  double x2Fr(int x) { return Fr.scr2d(x, rcG.left, rcG.right); }
  int Fr2x(double f) { return Fr.d2scr(f, rcG.left, rcG.right); }
  double y2Am(int y) { return Am.scr2d(y, rcG.bottom, rcG.top); }
  int Am2y(double a) { return Am.d2scr(a, rcG.bottom, rcG.top); }
  void SetAxis() {
    Fr.Set(st * FrScale, fi * FrScale, AS_HORIZ | 15);
    Am.Set(Amin, Amax, AS_VERT | 15);
  }
  //Window
  ~spectrum() override;
};

#endif
