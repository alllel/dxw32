#ifndef _TWND_H_
#define _TWND_H_
#include <vector>
#include <iostream>
#include <ranges>
#include <memory>

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

class Window : public std::enable_shared_from_this<Window> {
  static std::vector<std::shared_ptr<Window>> all_;

 public:
  static auto& All() { return all_; }
  HWND hWnd = nullptr;
  int LineWidth              = 7; // in 1/10pt
  char FontName[LF_FACESIZE] = "Arial";
  int FontHeight             = 80; // in 1/10pt
  WORD FontType              = 0;  // as in CHOOSEFONT
  Window()=default;
  virtual ~Window();
  virtual void Draw(HDC hdc, RECT& rc, DCtype t, RECT* rcUpd = nullptr) = 0;
  virtual bool Command(WPARAM cmd);
  virtual bool WinProc(Msg& M);
  virtual char* PicName() = 0;

  bool SelFont();
  bool SelLW();
  void ToClp();
  void ToFile();
  void ToPrinter();
  static std::shared_ptr<Window> GetWindow(HWND);
  void Destroy();
  void Create(MDICREATESTRUCT&);
};

template <typename T>
  requires std::is_base_of_v<Window, T>
inline auto
WindowIterator() {
  return std::views::transform(Window::All(), [](std::shared_ptr<Window>& w) { return std::dynamic_pointer_cast<T>(w); })
      | std::views::filter([](std::shared_ptr<T> const& w) { return w.get() != nullptr; });
}

#endif
