#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <commdlg.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <span>

DLGPROC(ImpDlg);
int R_Ascii(char* fname);

int
ReadAscii() {
  char fnames[4096];
  char fname[260], *f;
  char* s;

  OPENFILENAME opfn;
  opfn.lStructSize       = sizeof(opfn);
  opfn.hwndOwner         = hFrame;
  opfn.lpstrFilter       = "Text files(*.txt)\0*.txt\0\0All Files\0*.*\0";
  opfn.lpstrCustomFilter = nullptr;
  opfn.nFilterIndex      = 1;
  opfn.lpstrFile         = fnames;
  opfn.nMaxFile          = 4096;
  opfn.lpstrFileTitle    = nullptr;
  opfn.lpstrInitialDir   = nullptr;
  opfn.lpstrTitle        = "Import Ascii";
#ifndef _WIN32
#define OFN_EXPLORER 0
#endif
  opfn.Flags       = OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
  opfn.lpstrDefExt = nullptr;

  fnames[0] = 0;

  if (!GetOpenFileName(&opfn)) {
    return -1;
  }

  strcpy(fname, fnames);
  s = fnames + strlen(fnames) + 1;
  if (*s) {
    f  = fname + strlen(fname);
    *f = '\\';
    ++f;
    while (*s) {
      strcpy(f, s);
      R_Ascii(fname);
      s += strlen(s) + 1;
    }
  } else {
    R_Ascii(fname);
  }
  return 0;
}

int
R_Ascii(char* fname) {
  FILE* f = fopen(fname, "rt");
  int c;
  int info;
  char chname[20], unit[20], *s;
  float R;
  int angle;
  if (!f) {
    MessageBox(hFrame, "Can't open file", fname, MB_OK | MB_ICONEXCLAMATION);
    return -1;
  }
  BeginWait();
  float t, v;
  long np   = 0;
  long size = 1024;
  long i;
  HGLOBAL hT = GlobalAlloc(GMEM_MOVEABLE, size * sizeof(float));
  HGLOBAL hV = GlobalAlloc(GMEM_MOVEABLE, size * sizeof(float));
  float* T   = (float*) GlobalLock(hT);
  float* V   = (float*) GlobalLock(hV);
  c          = fgetc(f);
  if (c == EOF) {
    MessageBox(hFrame, "Empty file", fname, MB_OK | MB_ICONEXCLAMATION);
    return -1;
  }
  if (c == '#') {
    if (!fgets(buf, 128, f)) {
      MessageBox(hFrame, "Empty file", fname, MB_OK | MB_ICONEXCLAMATION);
      return -1;
    }
    if (sscanf(buf, "%s R%g A%d U:%s", chname, &R, &angle, unit) == 4) info = TRUE;
    else
      info = FALSE;
  } else {
    info = FALSE;
    ungetc(c, f);
  }
  while (fgets(buf, 128, f)) {
    if (sscanf(buf, "%g %g", &t, &v) != 2) break;
    if (size == np) {
      size += 1024;
      GlobalUnlock(hT);
      hT = GlobalReAlloc(hT, size * sizeof(float), 0);
      T  = (float*) GlobalLock(hT);
      GlobalUnlock(hV);
      hV = GlobalReAlloc(hV, size * sizeof(float), 0);
      V  = (float*) GlobalLock(hV);
    }
    T[np] = t;
    V[np] = v;
    np++;
  }
  fclose(f);
  EndWait();

  float rate = (T[np - 1] - T[0]) / (np - 1);
  float Vmin, Vmax, T0;
  HGLOBAL hDat;
  std::vector<short int> Dat;
  float dV;
  T0   = T[0];
  Vmin = Vmax = V[0];
  for (i = 1; i < np; ++i) {
    if (fabs(((T[i] - T[i - 1]) / rate) - 1) > 0.01) {
      MessageBox(hFrame, "Non-constant rate", nullptr, MB_OK);
      goto exit;
    }
    if (V[i] < Vmin) Vmin = V[i];
    if (V[i] > Vmax) Vmax = V[i];
  }
  if (i == np) {
    dV   = (Vmax - Vmin) / 0x7FFE;
    Dat.resize(np);
    for (i = 0; i < np; ++i) Dat[i] = static_cast<short int>((V[i] - Vmin) / dV);
  }
exit:
  GlobalUnlock(hT);
  GlobalFree(hT);
  GlobalUnlock(hV);
  GlobalFree(hV);
  if (Dat.empty()) return -1;
  Gauge* G           = new Gauge(nullptr);
  G->V0              = Vmin;
  G->dV              = dV;
  //G->nRates          = 1;
  G->Rates.resize(1);//           = new Gauge::Piece[1];
  G->Rates[0].Np     = np;
  G->Rates[0].rate   = rate;
  G->Rates[0].Tstart = T0;
  G->FilePos         = 0;
  G->count           = np;
  //G->hVal            = hDat;
  G->val=Dat;
  G->Setup();
  if (info) {
    strncpy(G->ChNum, chname, 5);
    G->Z0 = 0;
    strncpy(G->unit, unit, 3);
    G->Z2     = 0;
    G->radius = R;
    G->angle  = angle;
  } else {
    s = strrchr(fname, '\\');
    if (s) {
      strncpy(G->ID, s + 1, 16);
      G->Z1 = 0;
    } else {
      G->ID[0] = 0;
    }
    if (DLG(ID_IMP, ImpDlg, (LPARAM) G) == IDCANCEL) {
      delete G;
      return -1;
    }
  }
  MDICREATESTRUCT cs;
  cs.szClass = "DXWchannel";
  cs.hOwner  = hInst;
  cs.x       = CW_USEDEFAULT;
  cs.y       = CW_USEDEFAULT;
  cs.cx      = CW_USEDEFAULT;
  cs.cy      = CW_USEDEFAULT;
  cs.style   = WS_MINIMIZE;
  cs.lParam  = 0;
  cs.szTitle = G->WinTitle();
  G->Create(cs);
  if (!G->hWnd) {
    delete G;
  }
  return 0;
}

DLGPROC(ImpDlg) {
  static Gauge* G;
  switch (msg) {
    case WM_INITDIALOG:
      G = (Gauge*) lParam;
      if (G->ID[0]) {
        SetWindowText(hDlg, G->ID);
      }
      return TRUE;
    case WM_COMMAND:
      switch (wParam) {
        case IDOK: {
          unsigned short h = 0, min = 0, s = 0, d = 0, m = 0, y = 0;
          GetDlgItemText(hDlg, IDC_IMP_NAM, G->ChNum, 6);
          GetDlgItemText(hDlg, IDC_IMP_UNI, G->unit, 4);
          GetDlgItemText(hDlg, IDC_IMP_DAT, buf, 20);
          sscanf(buf, "%hd:%hd:%hd", &d, &m, &y);
          GetDlgItemText(hDlg, IDC_IMP_TIM, buf, 20);
          sscanf(buf, "%hd.%hd.%hd", &h, &min, &s);
          G->time.hour  = h;
          G->time.min   = min;
          G->time.sec   = s;
          G->date.day   = d;
          G->date.month = m;
          G->date.year  = y;
          char* p;
          GetDlgItemText(hDlg, IDC_IMP_GAU, buf, 10);
          G->number = (int) strtol(buf, &p, 10);
          G->g_type = *p;
          if (!G->g_type) G->g_type = ' ';
          GetDlgItemText(hDlg, IDC_IMP_LIN, buf, 10);
          G->angle = atoi(buf);
          GetDlgItemText(hDlg, IDC_IMP_RAD, buf, 10);
          G->radius = atof(buf);
        }
        case IDCANCEL:
          EndDialog(hDlg, wParam);
      }
      return TRUE;
  }
  return FALSE;
}
