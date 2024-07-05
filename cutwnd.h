#ifndef _CUTWND_H
#define _CUTWND_H

#include <windows.h>
#include "twnd.h"

class CutWnd : public Window {
 public:
  virtual int GetCutRC(RECT&)       = 0;
  virtual void SetSize(HWND, RECT&) = 0;
  void SetCursor(POINTS&);
  void StartCut();
  virtual void FinishCut(RECT&) = 0;
  virtual BOOL WinProc(Msg& M);
  virtual void Draw(HDC hdc, RECT& rc, DCtype t, RECT* rcUpd = nullptr) = 0;
  void DrawCutRect(HDC);
  explicit CutWnd(char t) : Window(t) {}
};

#endif
