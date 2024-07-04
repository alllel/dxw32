#ifndef _TWND_H_
#define _TWND_H_

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
  static HLOCAL hLast;

 public:
  HLOCAL hThis;
  HWND hWnd;
  char WinType;
  BOOL Destroy;
  int LineWidth; // in 1/10pt
  char FontName[LF_FACESIZE];
  int FontHeight; // in 1/10pt
  WORD FontType;  // as in CHOOSEFONT
  Window();
  virtual ~Window();
  virtual void AtUnlock();
  virtual void Draw(HDC hdc, RECT& rc, DCtype t, RECT* rcUpd = nullptr) = 0;
  virtual BOOL Command(WPARAM cmd);
  virtual BOOL WinProc(Msg& M);
  virtual char* PicName() = 0;

  BOOL SelFont();
  BOOL SelLW();
  void ToClp();
  void ToFile();
  void ToPrinter();
  static Window* LockWindow(HLOCAL);
  static Window* GetWindow(HWND);
  void Create(MDICREATESTRUCT&);
  void UnlockWindow();
  void* operator new(unsigned b, unsigned add = 0);
  void operator delete(void*);
};

inline Window*
Window::GetWindow(HWND hWnd) {
  return Window::LockWindow(GetWinHL(hWnd));
}

inline void*
Window::operator new(unsigned bytes, unsigned add) {
  hLast = LocalAlloc(LMEM_MOVEABLE, bytes + add);
  return LocalLock(hLast);
}

inline Window::Window() {
  Destroy   = FALSE;
  hThis     = hLast;
  hLast     = nullptr;
  hWnd      = nullptr;
  LineWidth = 7;
  lstrcpy(FontName, "Arial");
  FontHeight = 80;
  FontType   = 0;
}

inline void
Window::operator delete(void*) {
  LocalUnlock(hLast);
  LocalFree(hLast);
  hLast = nullptr;
}

inline void
Window::UnlockWindow() {
  AtUnlock();
  if (Destroy && ((LocalFlags(hThis) & LMEM_LOCKCOUNT) == 1)) delete this;
  else
    LocalUnlock(hThis);
}

inline Window*
Window::LockWindow(HLOCAL h) {
  return (Window*) LocalLock(h);
}

#endif
