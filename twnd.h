#ifndef _TWND_H_
#define _TWND_H_
#include <vector>
#include <iostream>

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
  int LineWidth              = 7; // in 1/10pt
  char FontName[LF_FACESIZE] = "Arial";
  int FontHeight             = 80; // in 1/10pt
  WORD FontType              = 0;  // as in CHOOSEFONT
  Window()=default;
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


#endif
