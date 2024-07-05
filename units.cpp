#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"

#define NOMATCH (0xFFFF)

static WPARAM
FindString(HWND hDlg, char* str) {
  WPARAM Index;
  long res = SendDlgItemMessage(hDlg, IDC_UN_LIST, LB_GETCOUNT, 0, 0);
  if (res == LB_ERR) return NOMATCH;
  for (Index = 0; Index < (WPARAM) res; ++Index) {
    SendDlgItemMessage(hDlg, IDC_UN_LIST, LB_GETTEXT, Index, (LPARAM) (LPSTR) buf);
    if (!strcmp(buf, str)) return Index;
  }
  return NOMATCH;
}

//ARGSUSED
DLGPROC(UnitProc) {
  //static Gauge*G=nullptr;
  WPARAM Index;
  switch (msg) {
    case WM_INITDIALOG:
      SendDlgItemMessage(hDlg, IDC_UN_N_UN, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "atm");
      SendDlgItemMessage(hDlg, IDC_UN_N_UN, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "bar");
      SendDlgItemMessage(hDlg, IDC_UN_N_UN, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "Pa");
      SendDlgItemMessage(hDlg, IDC_UN_N_UN, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "V");
      SetDlgItemText(hDlg, IDC_UN_O_UN, ((Gauge*) lParam)->unit);
      SetDlgItemText(hDlg, IDC_UN_N_UN, ((Gauge*) lParam)->unit);
      SetDlgItemText(hDlg, IDC_UN_O_RT, "1");
      SetDlgItemText(hDlg, IDC_UN_N_RT, "1");
      SetDlgItemText(hDlg, IDC_UN_O_SH, "0");
      SetDlgItemText(hDlg, IDC_UN_N_SH, "0");
      {
        for (auto G : GaugeIterator())  {
          if (!lstrcmp(G->unit, ((Gauge*) lParam)->unit)) SendDlgItemMessage(hDlg, IDC_UN_LIST, LB_ADDSTRING, 0, (LPARAM) (LPSTR) (G->ChNum));
        }
      }
      Index = FindString(hDlg, ((Gauge*) lParam)->ChNum);
      if (Index != NOMATCH)
        SendDlgItemMessage(hDlg, IDC_UN_LIST, LB_SETSEL, TRUE, MAKELPARAM(Index, 0));
      return TRUE;
    case WM_COMMAND:
      switch (wParam) {
        case IDC_UN_ALL: {
          SendDlgItemMessage(hDlg, IDC_UN_LIST, LB_SELITEMRANGE, TRUE, MAKELPARAM(0, nGauges - 1));
        } break;
        case IDOK: {
          double nr = 1, or_ = 1, ns = 0, os = 0;
          GetDlgItemDouble(hDlg, IDC_UN_N_RT, nr);
          GetDlgItemDouble(hDlg, IDC_UN_O_RT, or_);
          GetDlgItemDouble(hDlg, IDC_UN_N_SH, ns);
          GetDlgItemDouble(hDlg, IDC_UN_O_SH, os);
          {
            for (auto G : GaugeIterator())  {
              Index = FindString(hDlg, G->ChNum);
              if (Index == NOMATCH) continue;
              if (SendDlgItemMessage(hDlg, IDC_UN_LIST, LB_GETSEL, Index, 0) > 0) {
                G->V0 -= G->Zero_corr;
                G->Zero_corr = 0;
                G->dV        = G->dV * nr / or_;
                G->V0        = (G->V0 - os) * nr / or_ + ns;
                if (G->_Upper != G->_Lower) {
                  G->_Lower = (G->_Lower - os) * nr / or_ + ns;
                  G->_Upper = (G->_Upper - os) * nr / or_ + ns;
                  if (G->_Lower > G->_Upper) {
                    double tmp = G->_Lower;
                    G->_Lower  = G->_Upper;
                    G->_Upper  = tmp;
                  }
                }
                GetDlgItemText(hDlg, IDC_UN_N_UN, G->unit, 4);
                G->FreeI();
                G->Redraw();
                Changed = 1;
              }
            }
          } //endfor
        }
        case IDCANCEL:
          EndDialog(hDlg, wParam);
          break;
      }
      return TRUE;
  }
  return FALSE;
}

//ARGSUSED
DLGPROC(TShiftProc) {
  WPARAM Index;
  switch (msg) {
    case WM_INITDIALOG:
      SetDlgItemText(hDlg, IDC_TS_VAL, "0");
      {
        for (auto G : GaugeIterator())  {
          SendDlgItemMessage(hDlg, IDC_TS_LIST, LB_ADDSTRING, 0, (LPARAM) (LPSTR) (G->ChNum));
        }
      }
      Index = (WPARAM) SendDlgItemMessage(hDlg, IDC_TS_LIST, LB_FINDSTRING, 0, (LPARAM) (LPSTR) (((Gauge*) lParam)->ChNum));
      SendDlgItemMessage(hDlg, IDC_TS_LIST, LB_SETSEL, TRUE, MAKELPARAM(Index, 0));
      return TRUE;
    case WM_COMMAND:
      switch (wParam) {
        case IDC_TS_ALL: {
          SendDlgItemMessage(hDlg, IDC_TS_LIST, LB_SELITEMRANGE, TRUE, MAKELPARAM(0, nGauges - 1));
        } break;
        case IDOK: {
          double shift = 0;
          GetDlgItemDouble(hDlg, IDC_TS_VAL, shift);
          {
            for (auto G : GaugeIterator())  {
              long res = SendDlgItemMessage(hDlg, IDC_TS_LIST, LB_FINDSTRING, 0, (LPARAM) (LPSTR) (G->ChNum));
              if (res == LB_ERR) continue;
              Index = (WPARAM) res;
              if (SendDlgItemMessage(hDlg, IDC_TS_LIST, LB_GETSEL, Index, 0) > 0) {
                for (unsigned i = 0; i < G->nRates; ++i) G->Rates[i].Tstart += shift;
                G->Redraw();
                Changed = 1;
              }
            }
          } //endfor
        }
        case IDCANCEL:
          EndDialog(hDlg, wParam);
          break;
      }
      return TRUE;
  }
  return FALSE;
}
