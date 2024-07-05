#ifndef _GAUGE_H_
#define _GAUGE_H_

/******************************************************************************/
#include "axis.h"
#include "twnd.h"
#include "cutwnd.h"
#include "pline.h"
#include <dos.h>
#include <ranges>

/******************************************************************************/

class RTbyGaugeIterator;
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

struct Experiment {
  static int nExp;
  int count;
  char* Name;
  char* Dir;
  Experiment(char* N, char* D);
  ~Experiment();
  void Inc() { count++; }
  void Dec() {
    count--;
    if (!count) delete this;
  }
  char* File(const char* ext);
  char* IXC() { return File("IXC"); }
  char* DAT() { return File("DAT"); }
  int operator==(Experiment&);
};
// Creates Experiment after Get[Open/Save]FileName
Experiment* SetupExp();

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
    unsigned long Np;
    double rate;
    double Tstart;
  }* Rates;
  unsigned nRates;
  Experiment* Exp;
  // Modified parms
  double _Lower, _Upper;
  double Zero_corr;
  unsigned long start, final;
  // Calculated
  unsigned long FilePos;
  unsigned long count;
  long pts[2];
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
  HGLOBAL hVal, hImp;
  short int* val;
  float* imp;
  double Imax, Imin;
  // window specific
  Pline *PolyP, *PolyI;
  long Curr;
  BOOL rcValid, frcValid;
  fRECT fr;
  RECT rcGrf;
  BOOL drawI;
  //for rtdlg
  BOOL displayed;
  BOOL checked;

  // Methods
  // Constructors & destructors
  Gauge(Experiment* E);
  virtual ~Gauge();

  // Memory Management
  void LockD();
  void UnlockD();
  void FreeD();
  BOOL ReadD();

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
  virtual int GetCutRC(RECT& rc);
  virtual void FinishCut(RECT& rc);
  virtual void SetSize(HWND, RECT&);
  void SetInfo();

  // Menu commands & window messages
  int Write(HFILE, FILE*);
  void AfterWrite(Experiment*, long&);
  void AcceptZero();
  void AcceptZero2();
  void WriteASCII();
  void Restore();

  void Redraw();
  virtual void Draw(HDC, RECT&, DCtype, RECT*);
  virtual BOOL Command(WPARAM cmd);
  virtual BOOL WinProc(Msg& M);
  void Plot(HDC, DrOpt&);

  // Auxilary
  void Link(Experiment* E) {
    if (Exp) Exp->Dec();
    Exp = E;
    Exp->Inc();
  }
  char* FileName() { return Exp->DAT(); }
  char* WinTitle();
  void SetTitle() { SetWindowText(hWnd, WinTitle()); }
  virtual char* PicName();

  // Data access
  double I2D(short int v) { return V0 + v * dV - Zero_corr; }
  short int D2I(double v) { return (short int) ((v - V0 + Zero_corr) / dV); }

  double P2T(long);
  long T2P(double);

  double Val(long i);

  float& Imp(long i);

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

template <typename T>
  requires std::is_base_of_v<Window, T>
inline auto
WindowIterator() {
  return std::views::transform(Window::All(), [](Window* w) { return dynamic_cast<T*>(w); })
      | std::views::filter([](T* w) { return w != nullptr; });
}

inline auto
GaugeIterator() { return WindowIterator<Gauge>();}

#include "gaug_inl.h"

#endif
