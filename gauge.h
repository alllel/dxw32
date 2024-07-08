#ifndef _GAUGE_H_
#define _GAUGE_H_

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
  short hour;
  short min;
  short sec;
};
struct Date {
  short year;
  char month;
  char day;
};


class Gauge : public CutWnd {

 public:
  // IXC file date
  // Head
  double radius;
  int angle;
  int number;
  char g_type;

  char ChNum[5];
  char Z0;
  char ID[16];
  char Z1;
  double dV, V0;
  char unit[3];
  char Z2;
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
  double _Lower, _Upper;
  double Zero_corr;
  size_t start, final;
  // Calculated
  size_t FilePos;
  size_t count;
  size_t pts[2];
  double Ipos;
  double Ineg;
  double Itot;
  //data,Windows handles
  /*	struct Dat {
		enum _Dmax {MaxLen=0x4000};
		HGLOBAL hVal,hImp;
		int *val;
		float *Imp;
		Dat(){hVal=nullptr;hImp=nullptr;val=nullptr;Imp=nullptr;}
	} *data;
*/
  //HGLOBAL hVal, hImp;
  std::vector<short int> val;
  std::vector<float> imp;
  double Imax, Imin;
  // window specific
  std::unique_ptr<Pline> PolyP, PolyI;
  size_t Curr;
  BOOL rcValid, frcValid;
  fRECT fr;
  RECT rcGrf;
  BOOL drawI;
  //for rtdlg
  BOOL displayed;
  BOOL checked;

  // Methods
  // Constructors & destructors
  explicit Gauge(std::shared_ptr<Experiment> E);
  ~Gauge() override;

  // Memory Management
  void LockD();
  void UnlockD();
  void FreeD();
  bool ReadD();

  void LockI();
  void UnlockI();
  void FreeI();
  void CalcI();


  // Data preparation
  void Setup();
  void ULSetup();

  // Pointer manipulation
  void DrawPointer(HDC);
  void SetPointer(WORD);
  void MovePointer(int);
  void HomePointer(BOOL);
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
  BOOL Command(WPARAM cmd) override;
  BOOL WinProc(Msg& M) override;
  void Plot(HDC, DrOpt&);

  // Auxilary
  void Link(std::shared_ptr<Experiment> E) {
    Exp = std::move(E);
  }
  auto FileName() { return Exp->DAT(); }
  char* WinTitle();
  void SetTitle() { SetWindowText(hWnd, WinTitle()); }
  virtual char* PicName();

  // Data access
  double I2D(short int v) { return V0 + v * dV - Zero_corr; }
  short int D2I(double v) { return (short int) ((v - V0 + Zero_corr) / dV); }

  double P2T(size_t x);
  size_t T2P(double);

  double Val(size_t i);

  float& Imp(size_t i);

  double Pp(int i) { return Val(pts[i]); }
  double Tp(int i) { return P2T(pts[i]); }
  double Start() { return P2T(start); }
  double Final() { return P2T(final); }
  double Upper() {
    if (_Upper == _Lower) ULSetup();
    return _Upper;
  }
  double Lower() {
    if (_Upper == _Lower) ULSetup();
    return _Lower;
  }
};

Gauge* GaugeByWnd(HWND);
Gauge* GaugeByChNum(char*);


inline auto
GaugeIterator() { return WindowIterator<Gauge>();}

#include "gaug_inl.h"

#endif
