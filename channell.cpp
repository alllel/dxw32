#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include "rt.h"
#include <commdlg.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "spectrum.h"

void MoveCutFrame(POINTS& pt);
void CancelCut();

BOOL
Gauge::WinProc(Msg& M) {
  BOOL rez = CutWnd::WinProc(M);
  if (rez) return rez;
  switch (M.msg) {
    case WM_MDIACTIVATE:
      SetInfo();
      if (hCapWin) CancelCut();
      return FALSE;
    case WM_CLOSE:
      if (MessageBox(hFrame, "Do You really want delete this channell?",
                     "Close Channell", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2)
          != IDYES) {
        ShowWindow(hWnd, SW_MINIMIZE);
        return TRUE;
      } else {
        Changed = 1;
        for (RTbyGaugeIterator RT(this); RT; ++RT) {
          int i;
          for (i = 0; RT->Chns[i] != hThis; ++i);
          for (++i; i < RT->n; ++i) RT->Chns[i - 1] = RT->Chns[i];
          RT->n--;
          InvalidateRect(RT->hWnd, nullptr, TRUE);
        }
        return FALSE;
      }
    case WM_SIZE:
      rcValid = FALSE;
      if (PolyP) delete PolyP;
      PolyP = nullptr;
      if (PolyI) delete PolyI;
      PolyI = nullptr;
      return FALSE;
    case WM_KEYDOWN:
      switch (M.wParam) {
        case VK_LEFT:
        case VK_RIGHT:
          MovePointer(LOWORD(M.lParam) * (M.wParam == VK_RIGHT ? 1 : -1));
          return TRUE;
        case VK_HOME:
        case VK_END:
          HomePointer(M.wParam == VK_HOME);
          return TRUE;
        case VK_ESCAPE:
          PointerSet(-1);
          return TRUE;
        default:
          return FALSE;
      }
    case WM_CHAR:
      switch (M.wParam) {
        case '1':
          PointSet(0);
          return TRUE;
        case '2':
          PointSet(1);
          return TRUE;
        default:
          return FALSE;
      }
    case WM_LBUTTONDOWN:
      SetPointer(MAKEPOINTS(M.lParam).x);
      return TRUE;
    default:
      return FALSE;
  }
}

DLGPROC(UnitProc);
DLGPROC(TShiftProc);

BOOL
Gauge::Command(WPARAM cmd) {
  if (Window::Command(cmd)) return TRUE;
  switch (cmd) {
    case CM_REST:
      if (!hDig) Digitize();
      else
        SetFocus(hDig);
      break;
    case CM_ZERO:
      AcceptZero();
      SetInfo();
      break;
    case CM_ZERO2:
      AcceptZero2();
      SetInfo();
      break;
    case CM_WASCII:
      WriteASCII();
      break;
    case CM_RESTORE:
      Restore();
      SetInfo();
      break;
    case CM_UNIT: {
      DLG(ID_UNIT, UnitProc, (LPARAM) this);
    } break;
    case CM_TSHIFT: {
      DLG(ID_TSHIFT, TShiftProc, (LPARAM) this);
    } break;
    case CM_INFO:
      Info();
      break;
    case CM_IMP:
      drawI = !drawI;
      Redraw();
      SetInfo();
      break;
    case CM_FFT:
      (new spectrum(this))->UnlockWindow();
      break;
    default:
      return FALSE;
  }
  return TRUE;
}

void
Gauge::AcceptZero() {
  LockD();
  if (Curr != -1) {
    unsigned long F, T, i;
    double AVE = 0.0;
    F          = Curr - 10;
    T          = Curr + 10;
    if (F < start) F = start;
    if (T > final) F = final;
    for (i = F; i < T; ++i) AVE += Val(i);
    AVE /= T - F;
    Zero_corr += AVE;
    _Lower -= AVE;
    _Upper -= AVE;
    frcValid = FALSE;
    FreeI();
    Redraw();
    Changed = 1;
  }
}

void
Gauge::AcceptZero2() {
  if (pts[0] != -1 && pts[1] != -1) {
    double V, T0, T1, P0, P1;
    unsigned long i;
    LockD();
    T0 = Tp(0);
    T1 = Tp(1);
    P0 = Pp(0);
    P1 = Pp(1);
    for (i = start; i < final; ++i) {
      V = Val(i);
      V -= (P2T(i) - T0) / (T1 - T0) * (P1 - P0) + P0;
      *(val + i) = D2I(V);
    }
    ULSetup();
    UnlockD();
    hVal     = GlobalReAlloc(hVal, 0, GMEM_MODIFY | GMEM_MOVEABLE);
    frcValid = FALSE;
    FreeI();
    Redraw();
    Changed = 1;
  }
}

void
Gauge::Restore() {
  Setup();
  frcValid = FALSE;
  FreeI();
  Redraw();
}
