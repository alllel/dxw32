#ifndef _TWND_H_
#define _TWND_H_
#include <vector>

inline void
SetWinHL(HWND w, HLOCAL h) { SetWindowLong(w, 0, (LONG) h); }
inline HLOCAL
GetWinHL(HWND w) { return (HLOCAL) GetWindowLong(w, 0); }

struct Msg {
  UINT msg;
  WPARAM wParam;
  LPARAM lParam;
  LRESULT ret;
  Msg(UINT m, WPARAM w, LPARAM l) {
    msg    = m;
    wParam = w;
    lParam = l;
    ret    = 0;
  }
};

enum DCtype { WMF     = 0,
              Screen  = -1,
              Printer = -2 };

class Window {
  static std::vector<Window*> all_;

 public:
  static auto const& All() { return all_; }
  HWND hWnd = nullptr;
  char WinType;
  int LineWidth              = 7; // in 1/10pt
  char FontName[LF_FACESIZE] = "Arial";
  int FontHeight             = 80; // in 1/10pt
  WORD FontType              = 0;  // as in CHOOSEFONT
  explicit Window(char type) : WinType { type } {}
  virtual ~Window();
  virtual void Draw(HDC hdc, RECT& rc, DCtype t, RECT* rcUpd = nullptr) = 0;
  virtual BOOL Command(WPARAM cmd);
  virtual BOOL WinProc(Msg& M);
  virtual char* PicName() = 0;

  BOOL SelFont();
  BOOL SelLW();
  void ToClp();
  void ToFile();
  void ToPrinter();
  static Window* GetWindow(HWND);
  void Create(MDICREATESTRUCT&);
};

inline Window*
Window::GetWindow(HWND hWnd) {
  for (auto w : Window::all_)
    if (w->hWnd == hWnd) return w;
  return nullptr;
}

#endif
