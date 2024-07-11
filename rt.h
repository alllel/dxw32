#ifndef RT_H
#define RT_H

struct RtDlgState;

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
  std::vector<HWND> Chns;
  bool RemoveChn(const Gauge* G);
  void InitDlg(RtDlgState& dlg);
  int ReadDlg(RtDlgState& dlg);
  void Draw(HDC, RECT&, DCtype, RECT*) override;
  char* PicName() override;
  bool Command(WPARAM cmd) override;
  bool WinProc(Msg& M) override;
  void FindChannell(WORD);
  void Load(char* rt_fname);
  void Save(char* rt_fname);
  RT() = default;
  void Create();
};

inline auto
RTIterator() { return WindowIterator<RT>(); }

inline auto
RTbyGaugeIterator(Gauge const* g) {
  return RTIterator()
      | std::views::filter([g](std::shared_ptr<RT> const& rt) {
           return std::ranges::find(rt->Chns, g->hWnd) != rt->Chns.end();
         });
}

#endif
