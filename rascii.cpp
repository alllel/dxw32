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
#include <fstream>

DLGPROC(ImpDlg);
bool R_Ascii(std::string ascii_file);

int
ReadAscii() {
  char fnames[4096];
  char* f;
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

bool
R_Ascii(std::string ascii_file) {
  std::fstream f { ascii_file, std::ios::in };
  int c;
  bool info;
  char chname[20], unit[20], *s;
  float R;
  int angle;
  if (f.bad()) {
    MessageBox(hFrame, "Can't open file", ascii_file.c_str(), MB_OK | MB_ICONEXCLAMATION);
    return false;
  }
  BeginWait();
  std::vector<float> T;
  std::vector<float> V;
  c = f.peek();
  if (c == std::fstream::traits_type::eof()) {
    MessageBox(hFrame, "Empty file", ascii_file.c_str(), MB_OK | MB_ICONEXCLAMATION);
    return false;
  }
  if (c == '#') {
    f.getline(buf, std::size(buf), '\n');
    if (!f.gcount() || f.eof()) {
      MessageBox(hFrame, "Empty file", ascii_file.c_str(), MB_OK | MB_ICONEXCLAMATION);
      return false;
    }
    if (sscanf(buf, "%s R%g A%d U:%s", chname, &R, &angle, unit) == 4) info = true;
    else
      info = false;
  } else {
    info = false;
  }
  while (f.eof()) {
    f.getline(buf, std::size(buf), '\n');
    if (f.gcount() == 0) continue;
    if (buf[0] == '#') continue;
    float t, v;
    if (sscanf(buf, "%g %g", &t, &v) != 2) {
      MessageBox(hFrame, buf, "Read Ascii: unrecognised line", MB_OK | MB_ICONEXCLAMATION);
      break;
    }
    T.push_back(t);
    V.push_back(v);
  }
  f.close();
  EndWait();

  auto np = T.size();
  if (np < 2) {
    MessageBox(hFrame, "No data", "Read Ascii", MB_OK | MB_ICONEXCLAMATION);
    return false;
  }
  float rate = (T.back() - T.front()) / (static_cast<float>(np) - 1.0f);
  float Vmin, Vmax, T0;
  float dV;
  std::vector<short int> Dat;
  T0   = T[0];
  Vmin = Vmax = V[0];
  for (int i = 1; i < np; ++i) {
    if (fabs(((T[i] - T[i - 1]) / rate) - 1.0) > 0.01) {
      MessageBox(hFrame, "Non-constant rate", nullptr, MB_OK);
      return false;
    }
    if (V[i] < Vmin) Vmin = V[i];
    if (V[i] > Vmax) Vmax = V[i];
  }
  dV = (Vmax - Vmin) / 32000;
  Dat.resize(np);
  for (int i = 0; i < np; ++i) Dat[i] = static_cast<short int>((V[i] - Vmin) / dV);
  if (Dat.empty()) return false;
  auto G = std::make_shared<Gauge>(nullptr);
  G->V0  = Vmin;
  G->dV  = dV;
  //G->nRates          = 1;
  G->Rates.emplace_back(np, rate, T0);
  G->FilePos = 0;
  G->count   = np;
  //G->hVal            = hDat;
  G->val = Dat;
  G->Setup();
  if (info) {
    strncpy(G->ChNum, chname, 5);
    strncpy(G->unit, unit, 3);
    G->radius = R;
    G->angle  = angle;
  } else {
    auto name = std::filesystem::path { ascii_file }.stem().string();
    strncpy(G->ID, name.c_str(), 16);
    if (DLG(ID_IMP, ImpDlg, reinterpret_cast<LPARAM>(G.get())) == IDCANCEL) {
      G.reset();
      return false;
    }
  }
  G->Create();
  return true;
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
