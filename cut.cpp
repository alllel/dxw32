#define STRICT
#include <windows.h>
#include "dxw.h"
#include "twnd.h"
#include "cutwnd.h"
#include <cstdlib>

HPEN hpCut   = nullptr;
HWND hCapWin = nullptr;
int hResize  = 0;
int vResize  = 0;
static RECT rcCut;
static HWND hCutDlg = nullptr;
//Not necessary because smart callbacks used
//static FARPROC prDlg;

DLGPROC(CutSize);

void
CutWnd::SetCursor(POINTS& pt) {
  int x0, x1, xm;
  int y0, y1, ym;
  const char* Curs;
  if (!GetCutRC(rcCut)) {
    hResize = 0;
    vResize = 0;
    return;
  }
  x0 = rcCut.left;
  x1 = rcCut.right;
  xm = (x0 + x1) / 2;
  y1 = rcCut.top;
  y0 = rcCut.bottom;
  ym = (y0 + y1) / 2;
  if (abs(pt.x - x0) > 5) x0 = 0;
  if (abs(pt.x - x1) > 5) x1 = 0;
  if (abs(pt.x - xm) > 5) xm = 0;
  if (abs(pt.y - y0) > 5) y0 = 0;
  if (abs(pt.y - y1) > 5) y1 = 0;
  if (abs(pt.y - ym) > 5) ym = 0;
  if (x0 && y0) {
    hResize = -1;
    vResize = -1;
    Curs    = IDC_SIZENESW;
  } else if (x1 && y0) {
    hResize = 1;
    vResize = -1;
    Curs    = IDC_SIZENWSE;
  } else if (x0 && y1) {
    hResize = -1;
    vResize = 1;
    Curs    = IDC_SIZENWSE;
  } else if (x1 && y1) {
    hResize = 1;
    vResize = 1;
    Curs    = IDC_SIZENESW;
  } else if (x0 && ym) {
    hResize = -1;
    vResize = 0;
    Curs    = IDC_SIZEWE;
  } else if (x1 && ym) {
    hResize = 1;
    vResize = 0;
    Curs    = IDC_SIZEWE;
  } else if (xm && y0) {
    hResize = 0;
    vResize = -1;
    Curs    = IDC_SIZENS;
  } else if (xm && y1) {
    hResize = 0;
    vResize = 1;
    Curs    = IDC_SIZENS;
  } else {
    hResize = 0;
    vResize = 0;
    Curs    = IDC_ARROW;
  }
  ::SetCursor(LoadCursor(nullptr, Curs));
}

static void
DrawCut() {
  HDC hdc = GetDC(hCapWin);
  SelectObject(hdc, hpCut);
  SetROP2(hdc, R2_XORPEN);
  //Rectangle(hdc,rcCut.left,rcCut.top,rcCut.right,rcCut.bottom);
  MoveToEx(hdc, rcCut.left, rcCut.bottom, nullptr);
  LineTo(hdc, rcCut.left, rcCut.top);
  LineTo(hdc, rcCut.right, rcCut.top);
  LineTo(hdc, rcCut.right, rcCut.bottom);
  LineTo(hdc, rcCut.left, rcCut.bottom);
  ReleaseDC(hCapWin, hdc);
}

void
CutWnd::StartCut() {

  if (!GetCutRC(rcCut)) return;
  hCapWin = hWnd;
  SetCapture(hCapWin);
  CreateDialog(hInst, MAKEINTRESOURCE(ID_SIZE), hCapWin, CutSize /*prDlg*/);
  AlignWindow(hCutDlg, rcCut, vResize, -hResize);
  DrawCut();
}

void
MoveCutFrame(POINTS& pt) {
  DrawCut();
  if (hResize == 1) rcCut.right = pt.x;
  else if (hResize == -1)
    rcCut.left = pt.x;
  if (vResize == 1) rcCut.top = pt.y;
  else if (vResize == -1)
    rcCut.bottom = pt.y;
  DrawCut();
  Window* W  = Window::GetWindow(hCapWin);
  auto* CW = dynamic_cast<CutWnd*>(W);
  if (CW) CW->SetSize(hCutDlg, rcCut);
}

void
CancelCut() {
  DrawCut();
  DestroyWindow(hCutDlg);
  SetFocus(hCapWin);
  //Not necessary because smart callbacks used
  //FreeProcInstance(prDlg);
  ReleaseCapture();
  hCapWin = nullptr;
}

//ARGSUSED
DLGPROC(CutSize) {
  switch (msg) {
    case WM_INITDIALOG:
      hCutDlg = hDlg;
      return TRUE;
    case WM_DESTROY:
      hCutDlg = nullptr;
    default:
      return FALSE;
  }
}

BOOL
CutWnd::WinProc(Msg& M) {
  switch (M.msg) {
    case WM_MDIACTIVATE:
      if (hCapWin) CancelCut();
      return FALSE;
    case WM_MOUSEMOVE:
      if (!hCapWin) {
        SetCursor(MAKEPOINTS(M.lParam));
        return FALSE;
      } else {
        MoveCutFrame(MAKEPOINTS(M.lParam));
        return TRUE;
      }
    case WM_LBUTTONDOWN:
      if (!hCapWin) {
        SetCursor(MAKEPOINTS(M.lParam));
        if (hResize || vResize) StartCut();
        else
          return FALSE;
      }
      return TRUE;
    case WM_RBUTTONDOWN:
      if (hCapWin) {
        CancelCut();
        return TRUE;
      }
      return FALSE;
    case WM_LBUTTONUP:
      if (hCapWin) {
        CancelCut();
        FinishCut(rcCut);
        return TRUE;
      }
      return FALSE;
    default:
      return FALSE;
  }
}

inline void
Handle(HDC hdc, int x, int y) { Rectangle(hdc, x - 5, y - 5, x + 5, y + 5); }

void
CutWnd::DrawCutRect(HDC hdc) {
  RECT rcCut;
  GetCutRC(rcCut);
  SelectObject(hdc, hbrGray);
  Handle(hdc, rcCut.left, rcCut.top);
  Handle(hdc, rcCut.right, rcCut.top);
  Handle(hdc, rcCut.left, rcCut.bottom);
  Handle(hdc, rcCut.right, rcCut.bottom);
  Handle(hdc, (rcCut.left + rcCut.right) / 2, rcCut.top);
  Handle(hdc, (rcCut.left + rcCut.right) / 2, rcCut.bottom);
  Handle(hdc, rcCut.right, (rcCut.bottom + rcCut.top) / 2);
  Handle(hdc, rcCut.left, (rcCut.bottom + rcCut.top) / 2);
  SelectObject(hdc, hbrNull);
  Rectangle(hdc, rcCut.left, rcCut.top, rcCut.right, rcCut.bottom);
}
