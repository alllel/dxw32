#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <cstdlib>

void
Gauge::SetDigitize() {
  int i;
  if (!hDig) return;
  if (Curr != -1) {
    i = (int) SendDlgItemMessage(hDig, IDC_DIG_CHN, CB_FINDSTRING, -1, (LPARAM) (LPSTR) ChNum);
    SendDlgItemMessage(hDig, IDC_DIG_CHN, CB_SETCURSEL, i, 0);
    //	SetDlgItemText(hDig,IDC_DIG_CHN,ChNum);
    sprintf(buf, "%zd", Curr);
    SetDlgItemText(hDig, IDC_DIG_PNT, buf);
    sprintf(buf, "%.8lg", P2T(Curr));
    SetDlgItemText(hDig, IDC_DIG_TIM, buf);
    sprintf(buf, "%.8lg", Val(Curr));
    SetDlgItemText(hDig, IDC_DIG_VAL, buf);
  } else {
    SendDlgItemMessage(hDig, IDC_DIG_CHN, CB_SETCURSEL, -1, 0);
    SetDlgItemText(hDig, IDC_DIG_PNT, "");
    SetDlgItemText(hDig, IDC_DIG_TIM, "");
    SetDlgItemText(hDig, IDC_DIG_VAL, "");
  }
}

static std::shared_ptr<Gauge>
SelectedGauge() {
  GetDlgItemText(hDig, IDC_DIG_CHN, buf, 10);
  return GaugeByChNum(buf);
}

inline bool
Modified(int nItem) {
  return (bool) SendDlgItemMessage(hDig, nItem, EM_GETMODIFY, 0, 0);
}

//ARGSUSED
DLG_PROC(DigDlg) {
  bool ret = false;
  long i, n;
  double v, V;
  char* eptr;
  switch (msg) {
    case WM_INITDIALOG: {
      hDig = hDlg;
      {
        for (auto G : GaugeIterator()) {
          SendDlgItemMessage(hDlg, IDC_DIG_CHN, CB_ADDSTRING, 0, (LPARAM) (LPSTR) G->ChNum);
        }
      }
      HWND hChan = (HWND) SendMessage(hMDI, WM_MDIGETACTIVE, 0, 0);
      if (hChan) {
        auto G = GaugeByWnd(hChan);
        if (G) {
          G->LockD();
          if (G->Curr == -1) {
            HDC hdc = GetDC(hChan);
            G->DrawPointer(hdc);
            ReleaseDC(hChan, hdc);
          }
          G->SetDigitize();
        }
      }
      ret = true;
    } break;
    case WM_COMMAND:
      switch (wParam) {
        case IDCANCEL: {
          DestroyWindow(hDlg);
          ret = true;
        } break;
        case IDC_DIG_MIN: {
          auto G = SelectedGauge();
          if (G) {
            G->LockD();
            v = G->Upper();
            n = -1;
            for (i = G->start; i < G->final; ++i) {
              V = G->Val(i);
              if (V < v) {
                v = V;
                n = i;
              }
            }
            G->PointerSet(n);
          }
        } break;
        case IDC_DIG_MAX: {
          auto G = SelectedGauge();
          if (G) {
            G->LockD();
            v = G->Lower();
            n = -1;
            for (i = G->start; i < G->final; ++i) {
              V = G->Val(i);
              if (V > v) {
                v = V;
                n = i;
              }
            }
            G->PointerSet(n);
          }
        } break;
        case IDOK: {
          auto G = SelectedGauge();
          if (G) {
            i = -1;
            if (!Modified(IDC_DIG_PNT) && Modified(IDC_DIG_TIM)) {
              if (GetDlgItemText(hDlg, IDC_DIG_TIM, buf, 20)) {
                v = strtod(buf, &eptr);
                if (!*eptr) i = G->T2P(v);
              }
            } else {
              if (GetDlgItemText(hDlg, IDC_DIG_PNT, buf, 20)) {
                i = (int) strtol(buf, &eptr, 10);
                if (*eptr) i = -1;
              }
            }
            if (IsIconic(G->hWnd))
              SendMessage(hMDI, WM_MDIRESTORE, (WPARAM) G->hWnd, 0);
            G->PointerSet(i);
            SetFocus(G->hWnd);
          }
        } break;
        case IDC_DIG_PT1:
          [[fallthrough]];
        case IDC_DIG_PT2: {
          auto G = SelectedGauge();
          if (G) {
            G->PointSet(wParam == IDC_DIG_PT1 ? 0 : 1);
          }
        } break;
        default:
          break;
      }
      break;
    case WM_DESTROY:
      hDig = nullptr;
      break;
    default:
      break;
  }
  return ret;
}

void
Digitize() {
  if (!hDig) {
    if (Gauge::nGauges) {
      CreateDialog(hInst, MAKEINTRESOURCE(ID_DIGIT), hFrame, DigDlg /*hDigPr*/);
      AlignWindow(hDig, 0, 0, GetSystemMetrics(SM_CYSCREEN), GetSystemMetrics(SM_CXSCREEN), -1, 1);
    }
  } else {
    DestroyWindow(hDig);
  }
}
