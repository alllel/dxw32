#ifndef RT_H
#define RT_H

#include <algorithm>
typedef enum { Left  = 1,
               Right = -1,
               None  = 0 } state;
class RT : public Window {
 public:
  BOOL R_left : 1;
  state P_st : 2;
  BOOL T0_def : 1, T1_def : 1, R0_def : 1, R1_def : 1;
  BOOL clr : 1;
  double P_height, R_angle;
  double T0, T1, R0, R1;
  std::vector<Gauge*> Chns;
  bool RemoveChn(Gauge*);
  void InitDlg(HWND);
  int ReadDlg(HWND);
  void Draw(HDC, RECT&, DCtype, RECT*) override;
  char* PicName() override;
  BOOL Command(WPARAM cmd) override;
  BOOL WinProc(Msg& M) override;
  void FindChannell(WORD);
  void Load(char* fname);
  void Save(char* fname);
  RT();
};

inline auto
RTIterator() { return WindowIterator<RT>(); }

inline auto
RTbyGaugeIterator(Gauge* g) {
  return RTIterator() | std::views::filter([g](RT* rt) {
           return std::ranges::find(rt->Chns, g) != rt->Chns.end();
         });
}

extern RT* Rt;

#endif
