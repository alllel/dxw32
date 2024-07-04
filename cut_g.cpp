#define STRICT
#include <windows.h>
#include "dxw.h"
#include "cutwnd.h"
#include "gauge.h"
#include <cstdlib>

void
Gauge::FinishCut(RECT& rcCut) {
  long p;
  if (rcCut.left < rcCut.right) {
    p = T2P(scr2x(rcCut.left, fr, rcGrf));
    if (p < 0) p = 0;
    start = p;
    final = T2P(scr2x(rcCut.right, fr, rcGrf));
    if (final > count) final = count;
  }
  if (rcCut.bottom > rcCut.top) {
    _Lower = scr2y(rcCut.bottom, fr, rcGrf);
    _Upper = scr2y(rcCut.top, fr, rcGrf);
  }
  if (Curr < start || Curr >= final) Curr = -1;
  Changed = 1;
  FreeI();
  rcValid  = FALSE;
  frcValid = FALSE;
  Redraw();
  SetInfo();
}

void
Gauge::SetSize(HWND hCutDlg, RECT& rcCut) {
  sprintf(buf, "Start:%g", scr2x(rcCut.left, fr, rcGrf));
  SetDlgItemText(hCutDlg, IDC_SZ_ST, buf);
  sprintf(buf, "Final:%g", scr2x(rcCut.right, fr, rcGrf));
  SetDlgItemText(hCutDlg, IDC_SZ_FI, buf);
  sprintf(buf, "Upper:%g", scr2y(rcCut.top, fr, rcGrf));
  SetDlgItemText(hCutDlg, IDC_SZ_UP, buf);
  sprintf(buf, "Lower:%g", scr2y(rcCut.bottom, fr, rcGrf));
  SetDlgItemText(hCutDlg, IDC_SZ_LO, buf);
}

int
Gauge::GetCutRC(RECT& rc) {
  if (!(rcValid && frcValid)) return FALSE;

  rc.left   = x2scr(Start(), fr, rcGrf);
  rc.right  = x2scr(Final(), fr, rcGrf);
  rc.bottom = y2scr(Lower(), fr, rcGrf);
  rc.top    = y2scr(Upper(), fr, rcGrf);

  return TRUE;
}
