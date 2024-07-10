#ifndef RT_H
#define RT_H

#include <algorithm>
typedef enum { Left  = 1,
               Right = -1,
               None  = 0 } state;
class RT : public Window {
 public:
  bool R_left : 1 = true;
  state P_st : 2  = Right;
  bool T0_def : 1 = true;
  bool T1_def : 1 = true;
  bool R0_def : 1 = true;
  bool R1_def : 1 = true;
  bool clr : 1    = true;
  double P_height = 10.0, R_angle = 0.0;
  double T0 = 0.0, T1 = 0.0, R0 = 0.0, R1 = 0.0;
  std::vector<Gauge*> Chns;
  bool RemoveChn(Gauge*);
  void InitDlg(HWND);
  int ReadDlg(HWND);
  void Draw(HDC, RECT&, DCtype, RECT*) override;
  char* PicName() override;
  bool Command(WPARAM cmd) override;
  bool WinProc(Msg& M) override;
  void FindChannell(WORD);
  void Load(char* fname);
  void Save(char* fname);
  RT() = default;
};

inline auto
RTIterator() { return WindowIterator<RT>(); }

inline auto
RTbyGaugeIterator(Gauge* g) {
  return RTIterator() | std::views::filter([g](RT* rt) {
           return std::ranges::find(rt->Chns, g) != rt->Chns.end();
         });
}

//extern RT* Rt;

#endif
