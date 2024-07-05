#define STRICT
#include <windows.h>
#include <cstdlib>
#include "dxw.h"
#include "gauge.h"
#include <cstring>
#include <ctype.h>

DLGPROC(RenameDlg) {
  BOOL Trn;
  int shift;
  switch (msg) {
    case WM_INITDIALOG:
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDCANCEL:
          EndDialog(hDlg, 0);
          return TRUE;
        case IDOK:
          shift = GetDlgItemInt(hDlg, IDC_RENSHIFT, &Trn, TRUE);
          if (Trn) EndDialog(hDlg, shift);
          return TRUE;
      }
      return FALSE;
    default:
      return FALSE;
  }
}

void
RenumberChannels() {
  int shift = DLG(IDD_RENUMBER, RenameDlg, NULL);
  char Prefix[6], Suffix[6], Rez[20];
  int S, F, Num;
  if (shift) {
    for (auto G : GaugeIterator())  {
      for (S = 0; G->ChNum[S]; ++S) {
        if (isdigit(G->ChNum[S])) break;
      }
      for (F = S; G->ChNum[F]; ++F) {
        if (!isdigit(G->ChNum[F])) break;
      }
      if (F == S) continue;
      strcpy(Prefix, G->ChNum);
      Prefix[S] = 0;
      strcpy(Suffix, G->ChNum + F);
      G->ChNum[F] = 0;
      Num         = atoi(G->ChNum + S);
      sprintf(Rez, "%s%d%s", Prefix, Num + shift, Suffix);
      strncpy(G->ChNum, Rez, 5);
      G->SetTitle();
    }
    if (hDig) {
      Digitize();
      Digitize();
    }
    if (hInfo) {
      Info();
      Info();
    }
    SetFocus(hMDI);
  }
}
