#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include "rt.h"

RT* Rt = nullptr;

static void CreateRT(RT* Rt);

RT::RT() {
  R_left   = TRUE;
  P_st     = Right;
  T0_def   = TRUE;
  T1_def   = TRUE;
  R0_def   = TRUE;
  R1_def   = TRUE;
  clr      = TRUE;
  P_height = 10;
  R_angle  = 0;
}

struct LinesChk {
  int line;
  BOOL chk;
} Lines[GMAX];
int nLines = 0;

Gauge* iG[GMAX] = { nullptr };

int
FindLine(int line) {
  int i;
  for (i = 0; i < nLines; ++i)
    if (Lines[i].line == line) return i;
  return -1;
}

void
AddLine(int line) {
  int i = FindLine(line);
  if (i == -1) {
    Lines[nLines].line = line;
    Lines[nLines].chk  = FALSE;
    ++nLines;
  }
}

void
CheckLine(HWND hDlg, int ind, BOOL G_chk = TRUE) {
  SendDlgItemMessage(hDlg, IDC_RT_LINES, LB_SETSEL, TRUE, ind);
  Lines[ind].chk = TRUE;
  for (auto G : GaugeIterator()) {
    if (G->angle == Lines[ind].line) {
      G->displayed = TRUE;
      if (G_chk) G->checked = TRUE;
    }
  }
}

void
UnCheckLine(HWND hDlg, int ind) {
  SendDlgItemMessage(hDlg, IDC_RT_LINES, LB_SETSEL, FALSE, ind);
  Lines[ind].chk = FALSE;
  for (auto G : GaugeIterator()) {
    if (G->angle == Lines[ind].line) {
      G->displayed = FALSE;
      G->checked   = FALSE;
    }
  }
}

void
LinesList(HWND hDlg) {
  nLines = 0;
  {
    for (auto G : GaugeIterator()) {
      AddLine(G->angle);
    }
  }
  for (int i = 0; i < nLines; ++i) {
    SendDlgItemMessage(hDlg, IDC_RT_LINES, LB_ADDSTRING, 0,
                       (LPARAM) (LPSTR) itoa(Lines[i].line, buf, 10));
  }
}

void
ShowChns(HWND hDlg) {
  int j;
  if (SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_GETCOUNT, 0, 0)) {
    while (SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_DELETESTRING, 0, 0));
  }
  for (auto G : GaugeIterator()) {
    if (G->displayed) {
      sprintf(buf, "%s-%.2fm", G->ChNum, G->radius);
      j     = (int) SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_ADDSTRING, 0, (LPARAM) (LPSTR) buf);
      iG[j] = G;
      if (G->checked) {
        SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_SETSEL, TRUE, j);
      }
    }
  }
}

inline void
CMIN(double& n, double i) {
  if (i < n) n = i;
}
inline void
CMAX(double& n, double i) {
  if (i > n) n = i;
}

struct Unit {
  const char* name;
  double factor;
};
Unit ULen[] = {
  { "m", 1.0 },
  { "cm", 0.01 },
  { "mm", 0.001 },
  { nullptr, 0.0 }
};
Unit UTime[] = {
  { "s", 1.0 },
  { "ms", 0.001 },
  { "mks", 0.000001 },
  { nullptr, 0.0 }
};

void
SetDlgItemUnit(HWND hDlg, int ID, double Val, Unit* U) {
  int i;
  if (Val == 0.0) i = 0;
  else
    for (i = 0; U[i].name; ++i) {
      if (fabs(Val / U[i].factor) > 1.0) break;
    }
  if (!U[i].name) --i;
  sprintf(buf, "%g %s", Val / U[i].factor, U[i].name);
  SetDlgItemText(hDlg, ID, buf);
}

int
GetDlgItemUnit(HWND hDlg, int ID, double& val, Unit* U) {
  double w;
  char* p;
  int i;
  GetDlgItemText(hDlg, ID, buf, 30);
  if (!buf[0]) return FALSE;
  w = strtod(buf, &p);
  while (*p == ' ') ++p;
  if (*p == 0) {
    val = w * U[0].factor;
    return TRUE;
  }
  for (i = 0; U[i].name; ++i) {
    if (!stricmp(p, U[i].name)) {
      val = w * U[i].factor;
      return TRUE;
    }
  }
  return FALSE;
}

int
RT::ReadDlg(HWND hDlg) {
  int N, i;
  N = (int) SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_GETSELCOUNT, 0, 0);
  if (!N) return FALSE;
  N = (int) SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_GETCOUNT, 0, 0);
  for (i = 0; i < N; ++i) {
    if (SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_GETSEL, i, 0) <= 0) continue;
    SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_GETTEXT, i, (LPARAM) (LPSTR) buf);
    *strchr(buf, '-') = 0;
    Gauge* G          = GaugeByChNum(buf);
    if (G) {
      if (Chns.empty()) {
        R0 = G->radius;
        R1 = G->radius;
        T0 = G->Start();
        T1 = G->Final();
      } else {
        CMIN(R0, G->radius);
        CMAX(R1, G->radius);
        CMIN(T0, G->Start());
        CMAX(T1, G->Final());
      }
      Chns.push_back(G);
    }
  } //for(i
  R0_def = !GetDlgItemUnit(hDlg, IDC_RT_RMIN, R0, ULen);
  R1_def = !GetDlgItemUnit(hDlg, IDC_RT_RMAX, R1, ULen);
  T0_def = !GetDlgItemUnit(hDlg, IDC_RT_TMIN, T0, UTime);
  T1_def = !GetDlgItemUnit(hDlg, IDC_RT_TMAX, T1, UTime);

  GetDlgItemDouble(hDlg, IDC_RT_HGHT, P_height);
  GetDlgItemDouble(hDlg, IDC_RT_ANGL, R_angle);
  R_left = IsDlgButtonChecked(hDlg, IDC_RT_RL);
  clr    = IsDlgButtonChecked(hDlg, IDC_RT_CLR);
  if (IsDlgButtonChecked(hDlg, IDC_RT_PL)) P_st = Left;
  else if (IsDlgButtonChecked(hDlg, IDC_RT_PR))
    P_st = Right;
  else
    P_st = None;
  return TRUE;
}

void
RT::InitDlg(HWND hDlg) {
  int i, j;
  SetDlgItemDouble(hDlg, IDC_RT_ANGL, R_angle);
  SetDlgItemDouble(hDlg, IDC_RT_HGHT, P_height);
  CheckDlgButton(hDlg, R_left ? IDC_RT_RL : IDC_RT_RR, TRUE);
  CheckDlgButton(hDlg, IDC_RT_PL, P_st == Left);
  CheckDlgButton(hDlg, IDC_RT_PR, P_st == Right);
  CheckDlgButton(hDlg, IDC_RT_CLR, clr);
  if (!T0_def) SetDlgItemUnit(hDlg, IDC_RT_TMIN, T0, UTime);
  else
    SetDlgItemText(hDlg, IDC_RT_TMIN, "Auto");
  if (!T1_def) SetDlgItemUnit(hDlg, IDC_RT_TMAX, T1, UTime);
  else
    SetDlgItemText(hDlg, IDC_RT_TMAX, "Auto");
  if (!R0_def) SetDlgItemUnit(hDlg, IDC_RT_RMIN, R0, ULen);
  else
    SetDlgItemText(hDlg, IDC_RT_RMIN, "Auto");
  if (!R1_def) SetDlgItemUnit(hDlg, IDC_RT_RMAX, R1, ULen);
  else
    SetDlgItemText(hDlg, IDC_RT_RMAX, "Auto");
  {
    for (auto G : GaugeIterator()) {
      G->displayed = FALSE;
      G->checked   = FALSE;
    }
  }
  for (i = 0; i < nLines; ++i) {
    SendDlgItemMessage(hDlg, IDC_RT_LINES, LB_SETSEL, 0, i);
  }
  while (SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_DELETESTRING, 0, 0) > 0) {
  }
  for (auto G : Chns) {
    j          = FindLine(G->angle);
    G->checked = TRUE;
    CheckLine(hDlg, j, FALSE);
  }
  ShowChns(hDlg);
}

DLGPROC(RTdlg) {
  int i, j;
  switch (msg) {
    case WM_INITDIALOG: {
      if (!Rt) Rt = new RT;
      LinesList(hDlg);
      SendDlgItemMessage(hDlg, IDC_RT_ANGL, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "-45");
      SendDlgItemMessage(hDlg, IDC_RT_ANGL, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "-30");
      SendDlgItemMessage(hDlg, IDC_RT_ANGL, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "-15");
      SendDlgItemMessage(hDlg, IDC_RT_ANGL, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "0");
      SendDlgItemMessage(hDlg, IDC_RT_ANGL, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "15");
      SendDlgItemMessage(hDlg, IDC_RT_ANGL, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "30");
      SendDlgItemMessage(hDlg, IDC_RT_ANGL, CB_ADDSTRING, 0, (LPARAM) (LPSTR) "45");
      Rt->InitDlg(hDlg);
      SetFocus(GetDlgItem(hDlg, IDC_RT_LINES));
      return FALSE;
    } //case WM_INITDIALOG
    case WM_COMMAND: {
      WORD wNotCode = HIWORD(wParam);
      WORD idCtl    = LOWORD(wParam);
      switch (idCtl) {
        case IDC_RT_PL:
          if (wNotCode == BN_CLICKED) {
            if (IsDlgButtonChecked(hDlg, IDC_RT_PL))
              CheckDlgButton(hDlg, IDC_RT_PR, FALSE);
          }
          break;
        case IDC_RT_PR:
          if (wNotCode == BN_CLICKED) {
            if (IsDlgButtonChecked(hDlg, IDC_RT_PR))
              CheckDlgButton(hDlg, IDC_RT_PL, FALSE);
          }
          break;
        case IDC_RT_LINES: {
          switch (wNotCode) {
            case LBN_SELCHANGE: {
              i = (int) SendDlgItemMessage(hDlg, IDC_RT_LINES, LB_GETCARETINDEX, 0, 0);
              j = (SendDlgItemMessage(hDlg, IDC_RT_LINES, LB_GETSEL, i, 0) > 0);
              if (j != Lines[i].chk) {
                if (j) CheckLine(hDlg, i);
                else
                  UnCheckLine(hDlg, i);
                ShowChns(hDlg);
              }
            } //LBN_SELCHANGE
          } //switch(wNotCode)
        } break; //case IDC_RT_LINES
        case IDC_RT_CHNS: {
          switch (wNotCode) {
            case LBN_SELCHANGE: {
              i          = (int) SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_GETCARETINDEX, 0, 0);
              j          = (SendDlgItemMessage(hDlg, IDC_RT_CHNS, LB_GETSEL, i, 0) > 0);
              Gauge* G   = iG[i];
              G->checked = j;
            } //LBN_SELCHANGE
          } //switch(wNotCode)
        } break; //case IDC_RT_CHNS
        case IDC_SAVE_RT: {
          OFN ofn;
          ofn.lpstrFile       = fname;
          ofn.nMaxFile        = sizeof(fname);
          ofn.lpstrFilter     = "RT diagrams (*.rt)\0*.rt\0All Files\0*.*\0";
          ofn.lpstrInitialDir = Directory;
          ofn.Flags |= OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
          ofn.lpstrDefExt = "rt";
          fname[0]        = 0;
          if (!GetSaveFileName(&ofn)) break;
          {
            RT* tmp = new RT;
            tmp->ReadDlg(hDlg);

            strcpy(tmp->FontName, Rt->FontName);
            tmp->FontHeight = Rt->FontHeight;
            tmp->FontType   = Rt->FontType;
            tmp->LineWidth  = Rt->LineWidth;

            tmp->Save(fname);
            delete tmp;
          }
        } break;
        case IDC_LOAD_RT: {
          OFN ofn;
          ofn.lpstrFile       = fname;
          ofn.nMaxFile        = sizeof(fname);
          ofn.lpstrFilter     = "RT diagrams (*.rt)\0*.rt\0All Files\0*.*\0";
          ofn.lpstrInitialDir = Directory;
          ofn.Flags |= OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
          ofn.lpstrDefExt = "rt";
          fname[0]        = 0;
          if (!GetOpenFileName(&ofn)) break;
          {
            RT* tmp = new RT;
            tmp->Load(fname);

            strcpy(Rt->FontName, tmp->FontName);
            Rt->FontHeight = tmp->FontHeight;
            Rt->FontType   = tmp->FontType;
            Rt->LineWidth  = tmp->LineWidth;

            tmp->InitDlg(hDlg);
            delete tmp;
          }
        } break;
        case IDOK: {
          if (!Rt->ReadDlg(hDlg)) {
            j = MessageBox(hFrame, "No channels selected!", nullptr, MB_RETRYCANCEL);
            if (j == IDCANCEL) EndDialog(hDlg, 0);
          } else {
            if (!Rt->hWnd) CreateRT(Rt);
            else
              InvalidateRect(Rt->hWnd, nullptr, TRUE);
            Rt = nullptr;
            EndDialog(hDlg, 1);
          }
        } break; //case IDOK
        case IDCANCEL:
          if (!Rt->hWnd) delete Rt;
          Rt = nullptr;
          EndDialog(hDlg, 0);
          break;
      } //switch(wParam)
    } break; //case WM_COMMAND
  } //switch(msg)
  return 0;
}

void
CreateRT(RT* Rt) {
  MDICREATESTRUCT cs;
  cs.szClass = "DXWRT";
  cs.szTitle = "R-T diagram";
  cs.hOwner  = hInst;
  cs.x = cs.y = CW_USEDEFAULT;
  cs.cx       = 550;
  cs.cy       = 650;
  cs.style    = 0;
  cs.lParam   = 0;
  Rt->Create(cs);
}

void
RT::Load(char* fname) {
  int i;
  Gauge* G;

  GetPrivateProfileString("RT", "Tmin", "Auto", buf, 100, fname);
  T0_def = !strcmp(buf, "Auto");
  if (!T0_def) T0 = atof(buf);

  GetPrivateProfileString("RT", "Tmax", "Auto", buf, 100, fname);
  T1_def = !strcmp(buf, "Auto");
  if (!T1_def) T1 = atof(buf);

  GetPrivateProfileString("RT", "Rmin", "Auto", buf, 100, fname);
  R0_def = !strcmp(buf, "Auto");
  if (!R0_def) R0 = atof(buf);

  GetPrivateProfileString("RT", "Rmax", "Auto", buf, 100, fname);
  R1_def = !strcmp(buf, "Auto");
  if (!R1_def) R1 = atof(buf);

  GetPrivateProfileString("RT", "Height", "10", buf, 100, fname);
  P_height = atof(buf);
  if (P_height == 0.0) P_height = 10;

  GetPrivateProfileString("RT", "Angle", "0", buf, 100, fname);
  R_angle = atof(buf);

  GetPrivateProfileString("RT", "Color", "No", buf, 100, fname);
  clr = !strcmp(buf, "Yes");

  GetPrivateProfileString("RT", "Raxis", "Left", buf, 100, fname);
  R_left = !strcmp(buf, "Left");

  GetPrivateProfileString("RT", "Paxis", "Right", buf, 100, fname);
  if (strcmp(buf, "Left")) {
    if (strcmp(buf, "Right")) P_st = None;
    else
      P_st = Right;
  } else
    P_st = Left;

  GetPrivateProfileString("RT", "Nch", "0", buf, 100, fname);
  int n = atoi(buf);
  Chns.clear();
  for (i = 0; i < n; ++i) {
    char cnum[5];
    cnum[0] = 'C';
    itoa(i, cnum + 1, 10);
    GetPrivateProfileString("Channels", cnum, "---", buf, 100, fname);
    G       = GaugeByChNum(buf);
    Chns.push_back(G);
  }
  GetPrivateProfileString("RT", "FontName", "Arial", buf, 100, fname);
  strcpy(FontName, buf);

  GetPrivateProfileString("RT", "FontHeight", "8", buf, 100, fname);
  FontHeight = (int) (10 * atof(buf));

  FontType = 0;
  GetPrivateProfileString("RT", "FontBold", "0", buf, 100, fname);
  if (atoi(buf)) FontType |= BOLD_FONTTYPE;
  GetPrivateProfileString("RT", "FontItalic", "0", buf, 100, fname);
  if (atoi(buf)) FontType |= ITALIC_FONTTYPE;

  GetPrivateProfileString("RT", "LineWidth", ".7", buf, 100, fname);
  LineWidth = (int) (10 * atof(buf));
}

void
RT::Save(char* fname) {
  int i;
  Gauge* G;

  if (T0_def) strcpy(buf, "Auto");
  else
    gcvt(T0, 6, buf);
  WritePrivateProfileString("RT", "Tmin", buf, fname);
  if (T1_def) strcpy(buf, "Auto");
  else
    gcvt(T1, 6, buf);
  WritePrivateProfileString("RT", "Tmax", buf, fname);
  if (R0_def) strcpy(buf, "Auto");
  else
    gcvt(R0, 6, buf);
  WritePrivateProfileString("RT", "Rmin", buf, fname);
  if (R1_def) strcpy(buf, "Auto");
  else
    gcvt(R1, 6, buf);
  WritePrivateProfileString("RT", "Rmax", buf, fname);
  gcvt(P_height, 4, buf);
  WritePrivateProfileString("RT", "Height", buf, fname);
  gcvt(R_angle, 4, buf);
  WritePrivateProfileString("RT", "Angle", buf, fname);
  WritePrivateProfileString("RT", "Color", (clr ? "Yes" : "No"), fname);
  WritePrivateProfileString("RT", "Raxis", (R_left ? "Left" : "Right"), fname);
  WritePrivateProfileString("RT", "Paxis", (P_st == Left ? "Left" : (P_st == Right ? "Right" : "No")), fname);
  itoa(Chns.size(), buf, 10);
  WritePrivateProfileString("RT", "Nch", buf, fname);
  for (auto G : Chns) {
    buf[0] = 'C';
    itoa(i, buf + 1, 10);
    WritePrivateProfileString("Channels", buf, G->ChNum, fname);
  }

  WritePrivateProfileString("RT", "FontName", FontName, fname);
  WritePrivateProfileString("RT", "FontHeight", gcvt(FontHeight * 0.1, 4, buf), fname);
  WritePrivateProfileString("RT", "FontBold", (FontType & BOLD_FONTTYPE ? "1" : "0"), fname);
  WritePrivateProfileString("RT", "FontItalic", (FontType & ITALIC_FONTTYPE ? "1" : "0"), fname);
  WritePrivateProfileString("RT", "LineWidth", gcvt(LineWidth * 0.1, 4, buf), fname);
}
bool
RT::RemoveChn(Gauge* G) {
  auto it = std::ranges::find(Chns, G);
  if (it == Chns.end()) return false;
  Chns.erase(it);
  return true;
}
