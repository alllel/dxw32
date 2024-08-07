#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include "rt.h"
#include <commdlg.h>
#include <cstring>
#include "spectrum.h"

void MoveCutFrame(POINTS& pt);
void CancelCut();

bool
Gauge::WinProc(Msg& M) {
  bool rez = CutWnd::WinProc(M);
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
        for (auto rt : RTbyGaugeIterator(this)) {
          if (rt->RemoveChn(this))
            InvalidateRect(rt->hWnd, nullptr, TRUE);
        }
        return FALSE;
      }
    case WM_SIZE:
      rcValid = FALSE;
      PolyP.reset();
      PolyI.reset();
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

DLG_PROC(UnitProc);
DLG_PROC(TShiftProc);

bool
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
    case CM_FFT: {
      auto s = std::make_shared<spectrum>(this);
      if (!s->hWnd) {
        MessageBox(hFrame, "Can't create spectrum", "Can't create window", MB_OK);
      }
    } break;
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
    m_Lower -= AVE;
    m_Upper -= AVE;
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
    LockD();
    T0 = Tp(0);
    T1 = Tp(1);
    P0 = Pp(0);
    P1 = Pp(1);
    for (size_t i = start; i < final; ++i) {
      V = Val(i);
      V -= (P2T(i) - T0) / (T1 - T0) * (P1 - P0) + P0;
      val[i] = D2I(V);
    }
    ULSetup();

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
