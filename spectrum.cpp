#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include "spectrum.h"
#include <cmath>

void transform(double const* val, double* freq_re, double* freq_im, int len);

spectrum::spectrum(Gauge* g) : G { g } {
  long i, j, n;
  //double *A, *re, *im;
  n  = G->final - G->start;
  i  = 1;
  while (i < n) i <<= 1;
  N  = i;
  N  = N / 2;
  std::vector<double> A(N);
  std::vector<double> re(N / 2);
  std::vector<double> im(N / 2);
  if (N > n) {
    for (i = 0; i < N - n; ++i) A[i] = 0.0;
  } else {
    i = 0;
  }
  G->LockD();
  for (j = G->start; i < N; ++i, ++j) A[i] = G->Val(j);

  transform(A.data(), re.data(), im.data(), N);
  A.clear();
  abs.resize(1 + (N / 2));
  phase.resize(1 + (N / 2));
  LockData();
  abs[0]       = fabs(re[0]);
  abs[N / 2]   = fabs(im[0]);
  phase[0]     = atan2(re[0], 0);
  phase[N / 2] = atan2(im[0], 0);
  AbsMax       = 0;
  FrScale      = 1 / (G->Rates[0].rate * N);
  st           = 0;
  fi           = N / 2;
  for (i = 1; i < N / 2; ++i) {
    abs[i] = sqrt(re[i] * re[i] + im[i] * im[i]);
    if (abs[i] > AbsMax) AbsMax = abs[i];
    phase[i] = atan2(re[i], im[i]);
  }
  Amin = 0.0;
  Amax = AbsMax;
  SetAxis();
  st = 0;
  fi = N / 2;
  MDICREATESTRUCT cs;
  cs.szClass = "DXWspectr";
  strcpy(buf, G->ChNum);
  strcat(buf, " spectrum");
  cs.szTitle = buf;
  cs.hOwner  = hInst;
  cs.x = cs.y = cs.cx = cs.cy = CW_USEDEFAULT;
  cs.style                    = 0;
  cs.lParam                   = 0;
  Create(cs);
}

void
spectrum::LockData() {
  //if(!abs)abs=(double*)LocalLock(Abs);
  //if(!phase)phase=(double*)LocalLock(Phase);
}

void
spectrum::UnlockData() {
  /*if(abs){
	LocalUnlock(Abs);
	abs=nullptr;
}*/
  /*if(phase){
	LocalUnlock(Phase);
	phase=nullptr;
}*/
}


spectrum::~spectrum() {
  //int i;
  //AtUnlock();
  /*
LocalFree(Abs);
LocalFree(Phase);
*/
  //delete abs;
  //delete phase;
}

char*
spectrum::PicName() {
  sprintf(buf, "%lsfft%s", G->Exp->path.c_str(), G->ChNum);
  return buf;
}

bool
spectrum::Command(WPARAM cmd) {
  return Window::Command(cmd);
}

bool
spectrum::WinProc(Msg& M) {
  return CutWnd::WinProc(M);
}

void
spectrum::Draw(HDC hdc, RECT& rc, DCtype t, RECT* rcUpd) {
  long i;
  int x, y, w, h;
  LockData();
  w          = rc.right - rc.left;
  h          = rc.bottom - rc.top;
  rcG.top    = rc.top + 0.05 * h;
  rcG.bottom = rc.bottom - 0.1 * h;
  rcG.left   = rc.left + 0.1 * w;
  rcG.right  = rc.right - 0.05 * w;
  //	MoveToEx(hdc,rcG.left,rcG.bottom,nullptr);
  //	LineTo(hdc,rcG.right,rcG.bottom);
  strcpy(buf, Uscale(Fr.Draw(hdc, rcG, h * 0.01), "Hz", ""));
  Text(hdc, rcG.right, rcG.bottom - h * 0.01, buf, TA_RIGHT | TA_BASELINE);
  Am.Draw(hdc, rcG, w * 0.01);

  for (i = st; i < fi; ++i) {
    x = Fr2x(i * FrScale);
    y = Am2y(abs[i]);
    //		x=(rcG.right-rcG.left)*(i-st)/(fi-st)+rcG.left;
    //		y=rcG.bottom-(rcG.bottom-rcG.top)*abs[i]/AbsMax;
    MoveToEx(hdc, x, rcG.bottom, nullptr);
    LineTo(hdc, x, y);
  }
  if (t == Screen) DrawCutRect(hdc);
}

int
spectrum::GetCutRC(RECT& rc) {
  rc.top    = Am2y(Amax);
  rc.bottom = Am2y(Amin);
  rc.left   = Fr2x(st * FrScale);
  rc.right  = Fr2x(fi * FrScale);
  return 1;
}

void
spectrum::SetSize(HWND hCutDlg, RECT& rcCut) {
  sprintf(buf, "Start:%g", x2Fr(rcCut.left));
  SetDlgItemText(hCutDlg, IDC_SZ_ST, buf);
  sprintf(buf, "Final:%g", x2Fr(rcCut.right));
  SetDlgItemText(hCutDlg, IDC_SZ_FI, buf);
  sprintf(buf, "Upper:%g", y2Am(rcCut.top));
  SetDlgItemText(hCutDlg, IDC_SZ_UP, buf);
  sprintf(buf, "Lower:%g", y2Am(rcCut.bottom));
  SetDlgItemText(hCutDlg, IDC_SZ_LO, buf);
}

void
spectrum::FinishCut(RECT& rcCut) {
  if (rcCut.left < rcCut.right) {
    st = x2Fr(rcCut.left) / FrScale;
    if (st < 0) st = 0;
    fi = x2Fr(rcCut.right) / FrScale;
    if (fi > (N / 2)) fi = N / 2;
  }
  if (rcCut.bottom > rcCut.top) {
    Amin = y2Am(rcCut.bottom);
    if (Amin < 0) Amin = 0;
    Amax = y2Am(rcCut.top);
  }
  SetAxis();
  InvalidateRect(hWnd, nullptr, TRUE);
}
